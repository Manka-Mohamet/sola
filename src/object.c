#include <stdio.h>
#include <string.h>


#include "../include/memory.h"
#include "../include/object.h"
#include "../include/value.h"
#include "../include/vm.h"


#define ALLOCATE_OBJ(type, objectType)
	(type*)allocateObject(sizeof(type), objectType)



static Obj* allocateObject(size_t size, ObjType type)
{
	Obj* obj = (Obj*)reallocate(NULL, 0, size);
	obj->type = type;

	return obj;


}



static ObjString* allocateString(char* chars, int length)
{
	ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
	string->length = length;
	string->chars = chars;

	return string;

}

ObjString* takeString(char* chars, int length)
{

	return allocateString(chars, length);

}


ObjString* copyString(const char* chars, int length)
{
	ObjString* heapChars = ALLOCATE(char, length + 1);
	memcpy(heapChars, chars, length);
	heapChars[length] = '\0';

	return allocateString(heapChars, length);

}
