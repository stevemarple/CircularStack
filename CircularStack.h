#ifndef CIRCULARSTACK_H
#define CIRCULARSTACK_H

#include <stdint.h>

class CircularStack {
public:
  CircularStack(void* buf, uint16_t bufLen);

  inline uint16_t getLength(void) const;
  inline bool isEmpty(void) const;
  inline void clear(void);

  void write(uint8_t b);
  void write(const void* ptr, uint16_t len);

  // uint8_t read(void);
  uint16_t read(void* ptr, uint16_t len);
  uint16_t readFIFO(void* ptr, uint16_t len);
  uint16_t peek(void* ptr, uint16_t len) const;
  uint16_t peekFIFO(void* ptr, uint16_t len) const;

  
private:
  uint8_t *buffer;
  uint8_t *bufferEnd;
  uint16_t bufferLen;
  uint8_t *top;
  uint16_t length;
};

uint16_t CircularStack::getLength(void) const
{
  return length;
}

bool CircularStack::isEmpty(void) const
{
  return length == 0;
}

void CircularStack::clear(void)
{
  length = 0;
  top = buffer;
}


// Class to put blocks of memory into a circular stack
class CircularStackBlock {
public:
  inline CircularStackBlock(void* buf, uint16_t bufLen);
  inline void clear(void);
  // Cannot ready get number of blocks 
  inline uint16_t bytesUsed(void) const;
  
  bool isEmpty(void) const;
  void write(const void* ptr, uint16_t len);
  uint16_t read(void* ptr, uint16_t len);

private:
  CircularStack stack;
};


CircularStackBlock::CircularStackBlock(void* buf, uint16_t bufLen)
  : stack(buf, bufLen)
{
  ;
}


void CircularStackBlock::clear(void)
{
  stack.clear();
}

uint16_t CircularStackBlock::bytesUsed(void) const
{
  return stack.getLength();
}


#endif
