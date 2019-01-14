#MAX31855 library for any type of micrcontroller

### 1.Library for Maxim Integrated MAX31855 which can be ported on any type of uC.

### 2. File structure description

|FOLDER|SUBFOLDER|FILES	|Description
|------|--------|-------|--------------------------------------------------------------------
|MAX31855|	  | max31855.h | contains headers of that can be used to control MAX31855 chip
|MAX31855|    | max31855.c | contains source code that can be used to control MAX31855 chip

### 3. How to use it
#### 3.1. Prepare function which returns data from SPI (where MAX31855 is connected) in format:

```c
void SPI_GET_DATA(uint8_t * dataBufferPointer, uint8_t dataLen);
```
where:
uint8_t * dataBufferPointer 
- is pointer to a buffer which your SPI API must use to store received data 

and 

uint8_t dataLen 
- is maximum number of bytes that your SPI API can store in buffer. 

#### 3.2. Invoke init function with pointer to prepared function:

```c

void SPI_GET_DATA(uint8_t * dataBufferPointer, uint8_t dataLen);

[...]

MAX31855_init(SPI_GET_DATA);

```
this function returns STATUS struct which fild named libStatus should be set to MAX31855_LIB_INITIALIZED

#### 3.3. Invoke function that receives data from MAX31855:
```c
MAX31855_getDataFromChip();
```

#### 3.4. Now data are ready to read. For data format see max31855.h file
