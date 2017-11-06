SWIG-CSV contains the CSV module for SWIG parser. The CSV module generates the Data information of a class/structure.

Example
========

Suppose there is a class named `Bike`.

```C++
class Bike {
public:
	int color;		// color of the bike
	int gearCount;		// number of configurable gear
	Bike() {
		// bla bla
	}
	~Bike() {
		// bla bla
	}
	void operate() {
		// bla bla
	}
};
```

It contains data and functionality. The `data` are `color` and `number`. The `operation` is the functionality.

```
| member | type |
| ---    | ---  |
| color | data |
| gearCount | data |
| operate | function/logic |

The CSV output of the class above looks like the following,

```CSV
# This is SWIG generated the CSV metadata of classes/structures. It can be used to generate code using AWK.
# Class or Structure name | member name | member type | member kind

Bike|color|int|variable|public|
Bike|gearCount|int|variable|public|
Bike|operate|void|function|public|f().
```

The interface-file looks like the following,

```i
%module unused
%insert("csvbanner")
%{
# This is SWIG generated the CSV metadata of classes/structures. It can be used to generate code using AWK.
# Class or Structure name | member name | member type | member kind
%}


%include "bike.h"
```

Example usage(TODO)
===============

#### Plain Old Data serialization

[Plain Old Data](TODO)(POD) is the data part of the of the class. The CSV output can be used to generate C/C++ code that serialize the POD. [Here](TODO) is an example AWK script that does that.

[Here] is an example AWK script that converts POD to JSON data.

#### Get C/C++ variable by name

In C/C++, it is sometimes necessary to know all the variable names and get the value by names. [Here] is an AWK script that generates the get(variablename) method.

