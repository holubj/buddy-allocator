#include <cstring>
#include <cstdint>
#include <cassert>

using namespace std;

struct TBlock {
	TBlock * nextFree;
	TBlock * self;
	bool free;
	int size;
};

TBlock * g_Start;

int g_Blocks;
int g_Size;

TBlock * g_Free[32];

int powof2(int val)
{
	int i = 0;

	while(val >>= 1)
		i++;
	
	return i;
}

void addToList(TBlock * block, int l)
{
	if(!block)
		return;

	block->nextFree = g_Free[l];
	g_Free[l] = block;
}

void removeFromList(TBlock * block, int l)
{
	if(!block)
		return;

	TBlock * tmp = g_Free[l];

	if(block == tmp){
		g_Free[l] = block->nextFree;
		return;
	}

	while(tmp){
		if(tmp->nextFree == block)
			tmp->nextFree = block->nextFree;

		tmp = tmp->nextFree;
	}
}

void HeapInit(void * memPool, int memSize)
{
	int p = powof2(memSize);
	memSize = 1 << p;

	g_Start = (TBlock *) memPool;
	g_Start->size = memSize - sizeof(TBlock);
	g_Start->free = true;
	g_Start->self = g_Start;

	g_Blocks = 0;
	g_Size = g_Start->size;

	for(int i = 0; i < 32; i++)
		g_Free[i] = NULL;

	addToList(g_Start, p);
}

TBlock * divide(TBlock * block, int l)
{
	int size = ((block->size + sizeof(TBlock)) / 2) - sizeof(TBlock);

	removeFromList(block, l);

	block->free = true;
	block->size = size;
	block->self = block;

	TBlock * buddy;	
	buddy = (TBlock *) ((uint8_t *)block + sizeof(TBlock) + size);
	buddy->free = true;
	buddy->size = size;
	buddy->self = buddy;

	addToList(buddy, l - 1);

	return block;
}

void * HeapAlloc(int size)
{
	int l = powof2(size + sizeof(TBlock)) + 1;

	while(!g_Free[l] && l < 32)
		l++;

	if(l >= 32)
		return NULL;

	TBlock * tmp;
	tmp = g_Free[l];

	removeFromList(tmp, l);

	while((tmp->size + sizeof(TBlock)) / 2 >= size + sizeof(TBlock)){
		tmp = divide(tmp, l);
		l--;
	}

	tmp->free = false;
	tmp->self = tmp;

	g_Blocks++;

	return tmp + 1;
}

TBlock * findBuddy(TBlock * block, int l)
{
	long addr = ((uint8_t *) block - (uint8_t *) g_Start);

	return (TBlock *)((addr ^= (1 << l)) + (size_t) g_Start);
}

TBlock * merge(TBlock * block)
{
	TBlock * buddy;

	int l = powof2(block->size + sizeof(TBlock));

	buddy = findBuddy(block, l);

	if(!buddy->free || buddy->size != block->size)
		return NULL;

	if(block > buddy){
		TBlock * x = block;
		block = buddy;
		buddy = x;
	}

	removeFromList(block, l);
	removeFromList(buddy, l);

	block->size = block->size * 2 + sizeof(TBlock);
	block->free = true;
	block->self = block;

	addToList(block, l + 1);

	return block;
}

bool HeapFree(void * blk)
{
	TBlock * tmp = (TBlock *)((uint8_t *) blk - sizeof(TBlock));

	if(tmp->self == tmp){
		tmp->free = true;

		addToList(tmp, powof2(tmp->size + sizeof(TBlock)));

		while(tmp)
			if(tmp->size == g_Size)
				break;
			else
				tmp = merge(tmp);

		g_Blocks--;

		return true;
	}

	return false;
}

void HeapDone(int * pendingBlk)
{
	*pendingBlk = g_Blocks;
}

int main(int argc, char * argv[])
{
	uint8_t * p0, *p1, *p2, *p3, *p4;
	int	pendingBlk;
	static uint8_t memPool[3 * 1048576];

	HeapInit ( memPool, 2097152 );
	assert ( ( p0 = (uint8_t*) HeapAlloc ( 512000 ) ) != NULL );
	memset ( p0, 0, 512000 );
	assert ( ( p1 = (uint8_t*) HeapAlloc ( 511000 ) ) != NULL );
	memset ( p1, 0, 511000 );
	assert ( ( p2 = (uint8_t*) HeapAlloc ( 26000 ) ) != NULL );
	memset ( p2, 0, 26000 );
	HeapDone ( &pendingBlk );
	assert ( pendingBlk == 3 );


	HeapInit ( memPool, 2097152 );
	assert ( ( p0 = (uint8_t*) HeapAlloc ( 1000000 ) ) != NULL );
	memset ( p0, 0, 1000000 );
	assert ( ( p1 = (uint8_t*) HeapAlloc ( 250000 ) ) != NULL );
	memset ( p1, 0, 250000 );
	assert ( ( p2 = (uint8_t*) HeapAlloc ( 250000 ) ) != NULL );
	memset ( p2, 0, 250000 );
	assert ( ( p3 = (uint8_t*) HeapAlloc ( 250000 ) ) != NULL );
	memset ( p3, 0, 250000 );
	assert ( ( p4 = (uint8_t*) HeapAlloc ( 50000 ) ) != NULL );
	memset ( p4, 0, 50000 );
	assert ( HeapFree ( p2 ) );
	assert ( HeapFree ( p4 ) );
	assert ( HeapFree ( p3 ) );
	assert ( HeapFree ( p1 ) );
	assert ( ( p1 = (uint8_t*) HeapAlloc ( 500000 ) ) != NULL );
	memset ( p1, 0, 500000 );
	assert ( HeapFree ( p0 ) );
	assert ( HeapFree ( p1 ) );
	HeapDone ( &pendingBlk );
	assert ( pendingBlk == 0 );


	HeapInit ( memPool, 2359296 );
	assert ( ( p0 = (uint8_t*) HeapAlloc ( 1000000 ) ) != NULL );
	memset ( p0, 0, 1000000 );
	assert ( ( p1 = (uint8_t*) HeapAlloc ( 500000 ) ) != NULL );
	memset ( p1, 0, 500000 );
	assert ( ( p2 = (uint8_t*) HeapAlloc ( 500000 ) ) != NULL );
	memset ( p2, 0, 500000 );
	assert ( ( p3 = (uint8_t*) HeapAlloc ( 500000 ) ) == NULL );
	assert ( HeapFree ( p2 ) );
	assert ( ( p2 = (uint8_t*) HeapAlloc ( 300000 ) ) != NULL );
	memset ( p2, 0, 300000 );
	assert ( HeapFree ( p0 ) );
	assert ( HeapFree ( p1 ) );
	HeapDone ( &pendingBlk );
	assert ( pendingBlk == 1 );


	HeapInit ( memPool, 2359296 );
	assert ( ( p0 = (uint8_t*) HeapAlloc ( 1000000 ) ) != NULL );
	memset ( p0, 0, 1000000 );
	assert ( ! HeapFree ( p0 + 1000 ) );
	HeapDone ( &pendingBlk );
	assert ( pendingBlk == 1 );

	return 0;
}
