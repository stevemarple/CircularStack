#include <string.h>
#include <CircularStack.h>


CircularStack::CircularStack(void* buf, uint16_t bufLen)
  : buffer((uint8_t*)buf), bufferEnd((uint8_t*)buf + bufLen),
    bufferLen(bufLen),
    top((uint8_t*)buf), length(0)
{
  ;
}

void CircularStack::write(uint8_t b)
{
  if (top == bufferEnd)
    top = buffer;
  *top++ = b;
  if (length != bufferLen)
    ++length;
}

// TODO: is there any point in write returning a value?
void CircularStack::write(const void* src, uint16_t srcLen)
{
  uint8_t *ptr = (uint8_t*)src;
  if (srcLen >= bufferLen) {
    // No point in writing parts which will be overwritten
    ptr += (srcLen - bufferLen);
    srcLen = bufferLen;

    // Avoid unnecessary wrap and 2nd memcpy
    top = buffer;
    length = 0;
  }
  
  // Copy up until the end of the buffer (or sooner)
  uint16_t bbw = bufferEnd - top; // bytes before wrap
  uint16_t len1 = (srcLen < bbw ? srcLen : bbw);
  memcpy(top, ptr, len1);
  if (len1 == bbw)
    top = buffer; // Wrap pointer
  else
    top += len1;

  if (srcLen > bbw) {
    uint16_t len2 = srcLen - bbw;
    memcpy(top, ptr + len1, len2);
    top += len2; 
  }

  // Do the maths on length carefully to avoid overflow
  if (srcLen >= bufferLen - length) {
    // Full to the brim
    length = bufferLen;
  }
  else {
    // Number of bytes added is less than remaining space, so can add
    // srcLen to length and not get overflow since the results must be
    // less than bufferLen.
    length += srcLen;
  }
}

// Read back n bytes, last in first out
uint16_t CircularStack::read(void* dest, uint16_t destLen)
{
  uint8_t *ptr = (uint8_t*)dest;
  if (destLen > length)
    destLen = length;

  uint16_t n = destLen; // count down
  while (n) {
    --top;
    if (top < buffer)
      top += bufferLen;
    if (ptr != NULL)
      *ptr++ = *top;
    --n;
  }
  length -= destLen;
  return destLen;
  
  
  // uint16_t r = peek(dest, destLen);
  // top -= r;
  // if (top < buffer)
  //   top += bufferLen;
  // length -= r;
  // return r;
}

uint16_t CircularStack::readFIFO(void* dest, uint16_t destLen)
{
  if (destLen > length)
    destLen = length;
  
  if (dest != NULL) {
    uint8_t *ptr = (uint8_t*)dest;

    // Copy bytes 
    uint16_t bbw = top - buffer; // bytes before wrap
    if (destLen < bbw) {
      memcpy(ptr, top - destLen, destLen);
    }
    else {
      uint16_t len2 = bbw;
      uint16_t len1 = destLen - bbw;
      memcpy(ptr, bufferEnd - len1, len1);
      ptr += len1;
      memcpy(ptr, buffer, len2);
    }
  }
  
  top -= destLen;
  if (top < buffer)
    top += bufferLen;
  length -= destLen;

  return destLen;
}


// Peek n bytes, last in first out
uint16_t CircularStack::peek(void* dest, uint16_t destLen) const
{
  // Take a shallow copy
  CircularStack s = *this;
  return s.read(dest, destLen);
}

// Peek n bytes, first in first out
uint16_t CircularStack::peekFIFO(void* dest, uint16_t destLen) const
{
  // Take a shallow copy
  CircularStack s = *this;
  return s.readFIFO(dest, destLen);
}

bool CircularStackBlock::isEmpty(void) const
{
  uint16_t blockLen;
  if (stack.peekFIFO(&blockLen, sizeof(blockLen)) != sizeof(blockLen))
    return true; // Failed to read the block size
  
  return (stack.getLength() < (blockLen + sizeof(blockLen)));
}

void CircularStackBlock::write(const void* ptr, uint16_t len)
{
  stack.write(ptr, len);
  stack.write(&len, sizeof(len));
}

uint16_t CircularStackBlock::read(void* ptr, uint16_t len)
{
  // Read the number of bytes in the block
  uint16_t blockLen;
  if (stack.readFIFO(&blockLen, sizeof(blockLen)) != sizeof(blockLen))
    return 0; // Failed to read number of bytes in the block

  if (blockLen > len) {
    // Insufficient room in the supplied buffer but remove from the
    // stack.
    stack.readFIFO(NULL, blockLen);
    return 0; 
  }
  
  if (stack.readFIFO(ptr, blockLen) == blockLen)
    return blockLen;
  else
    return 0;
}

