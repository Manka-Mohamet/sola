#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "../include/scanner.h"
#include "../include/memory.h"
#include "../include/common.h"

typedef struct{
	char* start;
	char* current;
	int line;

}Scanner;

Scanner scanner;
TokenArray array;


void initScanner(char* source){
	scanner.start = source;
	scanner.current = source;
	scanner.line = 1;

}

static void initToken(TokenArray* array){
	array->count = 0;
	array->capacity = 0;
	array->tokens = NULL;
}



void writeToken(TokenArray* array, Token token){
	if (array->count + 1 > array->capacity){
		int capacity = GROW_CAPACITY(array->capacity);
		array->tokens  = GROW_ARRAY(Token, array->tokens, array->capacity,  capacity);

	}

	array->tokens[array->count] = token;
	array->count++;
}




void freeToken(TokenArray* array){

	FREE_ARRAY(Token, array->tokens, array->capacity);
	initToken(array);
}



typedef struct Keyword{

	char* name;
	int length;
	TokenType type;

}Keyword;


// table-kan waa reserved keywords in our language.
static Keyword keywords[] = {

	{"iyo", 3, TOKEN_AND},
	{"ama", 3, TOKEN_OR },
	{"hadii", 5, TOKEN_IF},
	{"halka", 5, TOKEN_WHILE},
	{"daabac", 6, TOKEN_PRINT},
	{"celi", 4, TOKEN_RETURN},
	{"qor", 3, TOKEN_INPUT},
	{"def", 3, TOKEN_FUN},
	{"class", 5, TOKEN_CLASS},
	{"kan", 3, TOKEN_THIS},
	{"dhaxlaa", 7, TOKEN_EXTENDS},
	{"kor", 3, TOKEN_SUPER},
	{"for", 3, TOKEN_FOR},
	{"kale", 3, TOKEN_ELSE},
	{"waxba", 5, TOKEN_NIL},
	{"qalad", 5, TOKEN_FALSE},
	{"sax", 3, TOKEN_TRUE},

	/* kani reserved keyword ma'han balse,
	waxa uu cadeynaa inuu file-ka dhamaadkiisa lajoogo.*/
	{NULL, 0, TOKEN_EOF},
};



//////////////////////
// HELPER FUNCTION //
////////////////////

static bool isAtEnd(){
	return *scanner.current == '\0';

}


static bool isDigit(char c){
	return c >= '0' && c <= '9';

}


static bool isAlpha(char c){

	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c  == '_';

}


static char peek(){

	return *scanner.current;

}



static char peekNext(){
	if (isAtEnd()) return '\0';
	return scanner.current[1];

}



static bool match(char expected){

	if(isAtEnd()) return false;
	if (*scanner.current != expected) return false;

	scanner.current++;

	return true;

}



static char advance(){
	scanner.current++;
	return scanner.current[-1];

}



static void comments(){
	while (peek() != '\n'){
		advance();
	}
	scanner.line++;
	advance();

}



static Token errorToken(char* message){

	Token token;

	token.type = TOKEN_ERROR;
	token.start = message;
	token.length = (int)strlen(message);
	token.line = scanner.line;

	return token;

}



static void unknownChar(){
	fprintf("charecter aan la aqoon.\n");
	exit(66);
}


static Token createToken(TokenType type){
        Token token;

        token.type = type;
        token.start = scanner.start;
        token.length = (int) (scanner.current - scanner.start);
        token.line = scanner.line;

        return token;
}




static Token doubleString(){

	while(peek() == '"' && !isAtEnd()){
		if ( peek() == '\n') scanner.line++;
		advance();
	}

	if (isAtEnd()) return errorToken("double string charecter aan xirnayn");

	advance();

	return createToken(TOKEN_STRING);

}




static Token singleString(){

        while(peek() == '\''  && !isAtEnd()){
                if ( peek() == '\n') scanner.line++;
                advance();
        }

        if (isAtEnd()) return errorToken("single string charecter aan xirnayn");

        advance();

        return createToken(TOKEN_STRING);

}


static Token number(){
	while (isDigit(peek())) advance();

	if (peek() == '.'){

		advance();

		while(isDigit(peek())) advance();
	}

	return createToken(TOKEN_NUMBER);

}



static TokenType identifierType(){

	int length = (int)(scanner.current - scanner.start);

	for (Keyword* keyword = keywords; keyword->name != NULL; keyword++){
	   // looking for is this reserved work or an identifier.
		if(length == keyword->length && memcmp(scanner.start, keyword->name, length)){
			return keyword->type;
		}
	}


	return TOKEN_IDENTIFIER;
}


static  Token identifier(){

	while(isAlpha(peek()) || isDigit(peek())) advance();

	return createToken(identifierType());


}



static void _scanTokens(){

     char c = advance();


     if (isDigit(c)){
	//handling  numbers like: int, float.
	writeToken(&array, number());
	return;

      }if (isAlpha(c)){
		// handling all identifiers like class, def, daabac variable names etc.
		writeToken(&array, identifier());
		return;
      }else{

	switch (c){

		//single charecter tokens
	   case '(':  writeToken(&array,  createToken(TOKEN_LEFT_PAREN));return;
	   case ')':  writeToken(&array, createToken(TOKEN_RIGHT_PAREN)); return;
	   case '{':  writeToken(&array,  createToken(TOKEN_LEFT_BRACE)); return;
	   case '}':  writeToken(&array,  createToken(TOKEN_RIGHT_BRACE)); return;
	   case ';':  writeToken(&array,  createToken(TOKEN_SEMICOLON)); return;
	   case ',':  writeToken(&array,  createToken(TOKEN_COMMA)); return;
	   case '.':  writeToken(&array,  createToken(TOKEN_DOT)); return;
	   case '-':  writeToken(&array,  createToken(TOKEN_MINUS)); return;
	   case '+':  writeToken(&array,  createToken(TOKEN_PLUS)); return;
	   case '*':  writeToken(&array,  createToken(TOKEN_STAR)); return;
	   case '/':  writeToken(&array,  createToken(TOKEN_SLASH)); return;



		// one or two charecters
	   case '!': writeToken(&array, createToken(match('=') ? TOKEN_BANG_EQUAL :  TOKEN_BANG)); return;
	   case '=': writeToken(&array, createToken(match('=') ? TOKEN_EQUAL_EQUAL :  TOKEN_EQUAL)); return;
	   case '<': writeToken(&array, createToken(match('=') ? TOKEN_LESS_EQUAL :  TOKEN_LESS));  return;
	   case '>': writeToken(&array, createToken(match('=') ? TOKEN_GREATER_EQUAL :  TOKEN_GREATER)); return;

		// whitespacess, tab, newlines and comments
	  case '\n': {
		scanner.line++;
		advance();
		return;
	 }
	  case '\t': advance(); return;
	  case ' ':  advance(); return;
	  case '\r': advance(); return;
	  case '#': comments(); break;

		// literals :  string, number, identifier
	  case '"': writeToken(&array, doubleString()); return;
	  case '\'': writeToken(&array, singleString()); return;

		// hadii charecter kale lasoo galiyo like: ♡, |, 
		// waxa uu report "invalid  charecter".
	  default:
		unknownChar();
		break;
	}
    }

}




TokenArray scannToken(){
	initToken(&array);

	while (!isAtEnd()){
		_scanTokens();

	}


	Token token = createToken(TOKEN_EOF);
	writeToken(&array, token);

	return array;

}

