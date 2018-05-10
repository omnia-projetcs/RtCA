#ifndef VERSION_H
#define VERSION_H

	//Software Status
	static const char STATUS[] =  "Alpha";
	static const char STATUS_SHORT[] =  "b";
	
	//Standard Version Type
	static const long MAJOR  = 0;
	static const long MINOR  = 8;
	static const long BUILD  = 850;
	static const long REVISION  = 21;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT  = 2890;
	#define RC_FILEVERSION 0,8,850,21
	#define RC_FILEVERSION_STRING "0, 8, 850, 21\0"
	static const char FULLVERSION_STRING [] = "0.8.850.21";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY  = 241;
	

#endif //VERSION_H
