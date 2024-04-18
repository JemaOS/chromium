# JemaOS accelerometer setup flags:

## jemaos-accel-config
source code:

```
const char kJemaOSConfig[6][3][2] ={
        {"x", "y", "z"},
        {"y", "x", "z"},
        {"z", "y", "x"},
        {"z", "x", "y"},
        {"x", "z", "y"},
        {"y", "z", "x"},
    };
```

type: int
range: 0~5
purpose: setting the axises mapping between kernel driver and chrome, enable jemaos accelerometer driver.
example: "--jemaos-accel-config=0"

## jemaos-accel-pattern
source code:

```
struct DataPattern {
  int data_size;
  int data_index[3];
  };

const struct DataPattern KJemaOSDataPattern[2] = {
    {6, {0, 1, 2}},
    {12, {0, 1, 3}},
  };
```
type: int
range: 0~1
purpose: 0: system original pattern.
         1: hid_accel_3d pattern, support surface pro3, etc.
example: "--jemaos-accel-pattern=1"

## jemaos-accel-revert-x
type: bool
purpose: revert X axis in chrome

## jemaos-accel-revert-y
as above

## jemaos-accel-revert-z
as above

## jemaos-accel-right-move
type: int
range: 4~12
purpose: some acceleromer driver use less 16-bit integer as data, if it use bit 16 to bit 4 represent 12bits integer, we should right move 4 bits to get the right data.
example: "--jemaos-accel-right-move=4"
