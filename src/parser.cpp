/*! \file parser.cpp
   A recursive descent parser with operator precedence.

   Author: Eric Van Wyk
   Modified: Robert Edge        
   Modified: Sophia Stembridge, Wen Chuan Lee
   This algorithm is based on the work of Douglas Crockford in "Top
   Down Operator Precedence", a chapter in the book "Beautiful Code".
   Mr. Crockford describes in his chapter how his work is based on an
   algorithm described Vaughan Pratt in "Top Down Operator
   Precedence", presented at the ACM Symposium on Principles of
   Programming Languages.

   Douglas Crockford's chapter is available at 
    http://javascript.crockford.com/tdop/tdop.html

   Vaughan Pratt's paper is available at 
    http://portal.acm.org/citation.cfm?id=512931

   These are both quite interesting works and worth reading if you
   find the problem of parsing to be an interesting one.

   Last modified: Nov. 20, 2014.

*/


#include "parser.h"
#include "scanner.h"
#include "extToken.h"
#include "ast.h"
#include <stdio.h>
#include <assert.h>
using namespace std ;

/*! Destructor for parser.*/
Parser::~Parser() {
    if (s) delete s ;

    ExtToken *extTokenToDelete ;
    currToken = tokens ;
    while (currToken) {
        extTokenToDelete = currToken ;
        currToken = currToken->next ;
        delete extTokenToDelete ;
    }

    Token *tokenToDelete ;
    Token *currScannerToken = stokens ;
    while (currScannerToken) {
        tokenToDelete = currScannerToken ;
        currScannerToken = currScannerToken->next ;
        delete tokenToDelete ;
    }

}


/*! Constructor for parser*/
Parser::Parser ( ) { 
    currToken = NULL; prevToken = NULL ; tokens = NULL; 
    s = NULL; stokens = NULL; 
}

ParseResult Parser::parse (const char *text) {
    assert (text != NULL) ;

    ParseResult pr ;
    try {
        s = new Scanner() ;
        stokens = s->scan (text) ;        
        tokens = extendTokenList ( this, stokens ) ;

        assert (tokens != NULL) ;
        currToken = tokens ;
        pr = parseProgram( ) ;
    }
    catch (string errMsg) {
        pr.ok = false ;
        pr.errors = errMsg ;
        pr.ast = NULL ;
    }
    return pr ;
}

//! Function for implementation and testing purposes only
void Parser::initialzeParser (const char* text) {

	// used during dev only to be removed in final product... :D 
	s = new Scanner();
	stokens = s->scan (text) ;        
        tokens = extendTokenList ( this, stokens ) ;

        assert (tokens != NULL) ;
}


/* 
 * !parse methods for non-terminal symbols
 * --------------------------------------
 */


/*! \fn ParseResult Parser::parseProgram ()
    \brief Begins parsing, attaches Root node for ast
*/ 
ParseResult Parser::parseProgram () {
    
    ParseResult pr ;
    // root
    // Program ::= varName '(' ')' '{' Stmts '}' 
    match(variableName) ;
    string name( prevToken->lexeme ) ;
    match(leftParen) ;
    match(rightParen) ;
    match(leftCurly);
    ParseResult prStmts = parseStmts() ;
    match(rightCurly);
    match(endOfFile) ;

    /* November 20 12:17AM this is commented out because it causes a seg fault ... -Lee
    //create a program */
	VarName *v = new VarName(name);
	Stmts *s = NULL;
	if (prStmts.ast)
	{
		s = dynamic_cast<Stmts *>(prStmts.ast); //empty statements is a subclass of stmts
		if(!s) throw ((string) "Bad cast of Stmts in parseProgram");
	}
	
	pr.ast = new Root(v,s);
    return pr ;
}


/*! \fn ParseResult Parser::parseMatrixDecl ()
    \brief identical purpose of parseDecl, handles special matrix syntax.
*/ 
ParseResult Parser::parseMatrixDecl () {
    ParseResult pr ;
    match(matrixKwd);
    match(variableName) ;

	VarName *name = new VarName(prevToken->lexeme);
    // Decl ::= 'Matrix' varName '[' Expr ',' Expr ']' varName ',' varName  '=' Expr ';'
    if(attemptMatch(leftSquare)){
        ParseResult firstPr = parseExpr(0);
		Expr *firstExpr = dynamic_cast<Expr *> (firstPr.ast);
		
       	//Expr* 
        match(comma);
        ParseResult secondPr = parseExpr(0);
		Expr *secondExpr = dynamic_cast<Expr *> (secondPr.ast);
    
        match(rightSquare);
		ParseResult parseVar1 = parseVariableName();
		VarName *var1 = dynamic_cast<VarName *>(parseVar1.ast);
		
        match(comma);
        ParseResult parseVar2 = parseVariableName();
        VarName *var2 = dynamic_cast<VarName *>(parseVar2.ast);
        
        match(assign);
        ParseResult prExpr = parseExpr(0);
        Expr *lastExpr = dynamic_cast<Expr *>(prExpr.ast);
        
        pr.ast = new LongMatrixDecl(name,var1,var2,firstExpr,secondExpr,lastExpr);
    }
    // Decl ::= 'Matrix' varName '=' Expr ';'
    else if(attemptMatch(assign)){
        ParseResult prExpr = parseExpr(0);
        Expr *aExpr = dynamic_cast<Expr *>(prExpr.ast);
        pr.ast = new MatrixDecl(name, aExpr);
    }
    else{
        throw ( (string) "Bad Syntax of Matrix Decl in in parseMatrixDecl" ) ;
    }   

    match(semiColon) ;

    return pr ;
}

/*! \fn ParseResult Parser::parseStandardDecl()
    \brief Parses standardDecl and make proper ast node
    Decl ::= integerKwd varName | floatKwd varName | stringKwd varName
*/ 
ParseResult Parser::parseStandardDecl(){
    ParseResult pr ;
    std::string kwd;
    //ParseResult prType = parseType() ; 
    if ( attemptMatch(intKwd) ) {
        // Type ::= intKwd 
        kwd = prevToken->lexeme;
    } 
    else if ( attemptMatch(floatKwd) ) {
        // Type ::= floatKwd}
      kwd = prevToken->lexeme;
	}
    else if ( attemptMatch(stringKwd) ) {
        // Type ::= stringKwd
      kwd = prevToken->lexeme;
    }
    else if ( attemptMatch(boolKwd) ) {
        // Type ::= boolKwd
      kwd =prevToken->lexeme;
    }
    match(variableName) ;
    VarName *var = new VarName (prevToken->lexeme);
      pr.ast = new SimpleDecl (kwd,var);  
    match(semiColon) ;
    return pr ;
}


/*! \fn ParseResult Parser::parseDecl ()
    \brief Parses declarations
*/
ParseResult Parser::parseDecl () {
    ParseResult pr ;
    // Decl :: Matrix variableName ....
    if(nextIs(matrixKwd)){
        pr =parseMatrixDecl();
    } 
    // Decl ::= Type variableName semiColon
    else{
        pr=parseStandardDecl();
    }
    return pr ;
}



/*! \fn ParseResult Parser::parseStmts ()
    \brief Parses Stmts
*/
ParseResult Parser::parseStmts () {
    ParseResult pr ;
    if ( ! nextIs(rightCurly) && !nextIs(inKwd)  ) {
        // Stmts ::= Stmt Stmts
        ParseResult prStmt = parseStmt() ;
	Stmt *stmt = dynamic_cast<Stmt *>(prStmt.ast) ;
        ParseResult prStmts = parseStmts() ;
	Stmts *stmts = dynamic_cast<Stmts *>(prStmts.ast) ;
	pr.ast = new StmtsSeq(stmt, stmts);
    }
    else {
        // Stmts ::= 
        // nothing to match.
	pr.ast = new EmptyStmts(); 
    }
    return pr ;
}


/*! \fn ParseResult Parser::parseStmt ()
    \brief Parses any kind of stmt
*/
ParseResult Parser::parseStmt () {
    ParseResult pr ;

    //Stmt ::= Decl
    if(nextIs(intKwd)||nextIs(floatKwd)||nextIs(matrixKwd)||nextIs(stringKwd)||nextIs(boolKwd)){
        ParseResult prDecl = parseDecl();
        Decl * decl = dynamic_cast<Decl *>(prDecl.ast);
        pr.ast = new DeclStmt(decl);
    }
    //Stmt ::= '{' Stmts '}'
    else if (attemptMatch(leftCurly)){
        ParseResult prStmts = parseStmts();
		Stmts* stmts = dynamic_cast<Stmts *>(prStmts.ast);
        match(rightCurly);
        pr.ast = new BlockStmt(stmts);
    }   
    //Stmt ::= 'if' '(' Expr ')' Stmt
    //Stmt ::= 'if' '(' Expr ')' Stmt 'else' Stmt
    else if (attemptMatch(ifKwd)){
        match(leftParen);
        ParseResult prExpr = parseExpr(0);
        Expr* expr = dynamic_cast<Expr *>(prExpr.ast);
        match(rightParen);
        ParseResult prStmt = parseStmt();
		Stmt* stmt = dynamic_cast<Stmt *>(prStmt.ast);
        
        pr.ast = new IfStmt(expr,stmt);
        
        if(attemptMatch(elseKwd)){
            ParseResult prStmt2 = parseStmt();
            Stmt* stmt2 = dynamic_cast<Stmt *>(prStmt.ast);
            pr.ast = new IfElseStmt(expr,stmt,stmt2);
        }

    }
    //Stmt ::= varName '=' Expr ';'  | varName '[' Expr ',' Expr ']' '=' Expr ';'
    else if  ( attemptMatch (variableName) ) {
		VarName *var = new VarName(prevToken->lexeme);
        if (attemptMatch ( leftSquare ) ) {
              
              ParseResult prExpr = parseExpr(0);
              Expr *expr1 = dynamic_cast<Expr *>(prExpr.ast);

              match ( comma ) ;
              ParseResult prExpr2 = parseExpr(0);
              Expr *expr2 = dynamic_cast<Expr *>(prExpr2.ast);
              
              match  ( rightSquare ) ;
              match(assign);
              ParseResult prExpr3 = parseExpr(0);
			  Expr *expr3 = dynamic_cast<Expr *>(prExpr3.ast);
			
			
              pr.ast = new LongAssignStmt(var,expr1,expr2,expr3);
              match (semiColon);
            
        }
        else {
			//VarName *var = new VarName(prevToken->lexeme);
			match(assign);
			ParseResult prExpr = parseExpr(0);
	
            Expr *expr1 = dynamic_cast<Expr *>(prExpr.ast);
    
            pr.ast = new AssignStmt(var,expr1);
			match(semiColon);
		}
    }
    //Stmt ::= 'print' '(' Expr ')' ';'
    else if ( attemptMatch (printKwd) ) {
        match (leftParen) ;
        ParseResult prExpr = parseExpr(0);
        Expr *expr1 = dynamic_cast<Expr *>(prExpr.ast);
            
        match (rightParen) ;
        match (semiColon) ;
		pr.ast = new PrintStmt(expr1);
    }
    //Stmt ::= 'for' '(' varName '=' Expr ':' Expr ')' Stmt
    else if ( attemptMatch (forKwd) ) {
        match (leftParen) ;
        match (variableName) ;
        
        VarName *name = new VarName(prevToken->lexeme);
        
        match (assign) ;
        ParseResult prExpr = parseExpr(0);
        Expr *expr1 = dynamic_cast<Expr *>(prExpr.ast);
        
        
        match (colon) ;
        ParseResult prExpr2 = parseExpr(0);
        Expr *expr2 = dynamic_cast<Expr *>(prExpr2.ast);
        
        match (rightParen) ;
        
        
        ParseResult prStmt = parseStmt();
		Stmt* stmt = dynamic_cast<Stmt *>(prStmt.ast);
        pr.ast = new ForStmt(name,expr1,expr2,stmt);
    }
    //Stmt ::= 'while' '(' Expr ')' Stmt
    else if (attemptMatch(whileKwd)) {
        match(leftParen);
        ParseResult prExpr = parseExpr(0);
        Expr *expr1 = dynamic_cast<Expr *>(prExpr.ast);
        
        match(rightParen);
        ParseResult prStmt = parseStmt();
		Stmt* stmt = dynamic_cast<Stmt *>(prStmt.ast);
        pr.ast = new WhileStmt(expr1,stmt);
    }
    //Stmt ::= ';
    else if ( attemptMatch (semiColon) ) {
        // parsed a skip
    }
    else{
        throw ( makeErrorMsg ( currToken->terminal ) + " while parsing a statement" ) ;
    }
    // Stmt ::= variableName assign Expr semiColon
    
    return pr ;
}



/*! \fn ParseResult Parser::parseExpr (int rbp)
    \brief Calls appropriate parse functions for expressiont
*/
ParseResult Parser::parseExpr (int rbp) {
    /*! Examine current token, without consuming it, to call its
       associated parse methods.  The ExtToken objects have 'nud' and
       'led' methods that are dispatchers that call the appropriate
       parse methods.*/
    ParseResult left = currToken->nud() ;
    //cout << "rbp is:" << rbp <<endl;
    while (rbp < currToken->lbp() ) {
	//	cout << currToken->lbp() <<endl;
        left = currToken->led(left) ;
    }

    return left ;
}


/*
 * parse methods for Expr productions
 * ----------------------------------
 */

 //! Expr ::= trueKwd
 ParseResult Parser::parseTrueKwd ( ) {
     ParseResult pr ;
     match ( trueKwd ) ;
     pr.ast = new AnyConst(prevToken->lexeme);
     return pr ;
 }

 //! Expr ::= falseKwd
 ParseResult Parser::parseFalseKwd ( ) {
     ParseResult pr ;
     match ( falseKwd ) ;
     pr.ast = new AnyConst(prevToken->lexeme);
     return pr ;
 }

//! Expr ::= intConst
ParseResult Parser::parseIntConst ( ) {
    ParseResult pr ;
    match ( intConst ) ;
    pr.ast = new AnyConst(prevToken->lexeme);   ////////////////// this was where the first seg fault was caused. 
    return pr ;
}

//! Expr ::= floatConst
ParseResult Parser::parseFloatConst ( ) {
    ParseResult pr ;
    match ( floatConst ) ;
    pr.ast = new AnyConst(prevToken->lexeme);
    return pr ;
}

//! Expr ::= stringConst
ParseResult Parser::parseStringConst ( ) {
    ParseResult pr ;
    match ( stringConst ) ;
    pr.ast = new AnyConst(prevToken->lexeme);
    return pr ;
}

//! Expr ::= variableName .....
ParseResult Parser::parseVariableName ( ) {
    ParseResult pr ;
    match ( variableName ) ;
    std::string name(prevToken->lexeme);
	VarName *var = new VarName(name);
    if(attemptMatch(leftSquare)){
        ParseResult prExpr1 = parseExpr(0);
        match(comma);
        ParseResult prExpr2 = parseExpr(0);
        match(rightSquare);
	Expr *expr1 = dynamic_cast<Expr *>(prExpr1.ast);
	Expr *expr2 = dynamic_cast<Expr *>(prExpr2.ast);
	pr.ast = new MatrixRefExpr(var,expr1,expr2);
    }
    //! Expr ::= varableName '(' Expr ')'        //NestedOrFunctionCall
    else if(attemptMatch(leftParen)){
        ParseResult prExpr = parseExpr(0);
        match(rightParen);
	Expr *expr = dynamic_cast<Expr *>(prExpr.ast);
	pr.ast = new NestOrFuncExpr(var,expr);
    }
    //! Expr := variableName
    else{
        // variable 
        pr.ast = new VarName(name);
    }
    return pr ;
}


//! Expr ::= leftParen Expr rightParen
ParseResult Parser::parseNestedExpr ( ) {
    ParseResult pr ;
    match ( leftParen ) ;
    //parseExpr(0) ;
	ParseResult prCur = parseExpr(0);   					//TODO
	Expr *curExpr = dynamic_cast<Expr *>(prCur.ast);
    //pr.ast = new ParenExpr(prevToken->lexeme);
    match(rightParen) ;
	pr.ast = new ParenExpr(curExpr);
    return pr ;
}

//! Expr ::= 'if' Expr 'then' Expr 'else' Expr  
ParseResult Parser::parseIfExpr(){  
     ParseResult pr ; 
    match(ifKwd);
    ParseResult prExpr = parseExpr(0);
    Expr *ifExpr = dynamic_cast<Expr *>(prExpr.ast);
    match(thenKwd);
    prExpr = parseExpr(0);
    Expr *thenExpr = dynamic_cast<Expr *>(prExpr.ast);
    
    match(elseKwd);
    prExpr = parseExpr(0);
    Expr *elseExpr = dynamic_cast<Expr *>(prExpr.ast);
    pr.ast = new IfElseExpr(ifExpr,thenExpr,elseExpr);
    return pr;
}


//! Expr ::= 'let' Stmts 'in' Expr 'end' 
ParseResult Parser::parseLetExpr(){
   ParseResult pr ;
   match(letKwd);
   ParseResult prStatements = parseStmts();
   Stmts *statements = dynamic_cast<Stmts *>(prStatements.ast);
   match(inKwd);
   ParseResult prExpr = parseExpr(0);
   Expr *expr = dynamic_cast<Expr *>(prExpr.ast);
   match(endKwd);
   pr.ast = new LetExpr(statements,expr);
   return pr;
}

//! Expr ::= '!' Expr 
ParseResult Parser::parseNotExpr () {
    ParseResult pr ;
    match ( notOp ) ;
    ParseResult prNot = parseExpr (0);
    Expr *expr = dynamic_cast<Expr *>(prNot.ast);
    pr.ast = new NotExpr(expr);
    return pr ;

}

//! Expr ::= Expr plusSign Expr
ParseResult Parser::parseAddition ( ParseResult prLeft ) {
  // parser has already matched left expression 
  ParseResult pr ;
  //we added this
  Expr *left = dynamic_cast<Expr *> (prLeft.ast);
  //if (!left) throw( (string) "bad cast in left expr in parseAddition");


  match ( plusSign ) ;
  string * op = new string(prevToken->lexeme);
    
  ParseResult prRight = parseExpr( prevToken->lbp() ); 
  Expr *right = dynamic_cast<Expr *>(prRight.ast);
  //if(!right) throw((string) "Bad cast in right expr in parseAddition");

  pr.ast = new BinOpExpr(left, op, right);

  return pr ;
}

//! Expr ::= Expr star Expr
ParseResult Parser::parseMultiplication ( ParseResult prLeft ) {
  // parser has already matched left expression 
  ParseResult pr ;
  Expr *left = dynamic_cast<Expr *> (prLeft.ast);
  match ( star ) ;
  string* op = new string(prevToken->lexeme);
  ParseResult prRight = parseExpr (prevToken->lbp());
  Expr *right = dynamic_cast<Expr *>(prRight.ast);

  //parseExpr( prevToken->lbp() );
  pr.ast = new BinOpExpr (left, op,right); //////////////////the rest of the BinOps follow this as well. 
  return pr ;
}

//! Expr ::= Expr dash Expr
ParseResult Parser::parseSubtraction ( ParseResult prLeft ) {
  // parser has already matched left expression 
  ParseResult pr ;
  Expr *left = dynamic_cast<Expr *> (prLeft.ast);
  match ( dash ) ;
  string* op = new string(prevToken->lexeme);
  ParseResult prRight = parseExpr (prevToken->lbp());
  Expr *right = dynamic_cast<Expr *>(prRight.ast);

  //parseExpr( prevToken->lbp() );
  pr.ast = new BinOpExpr (left, op,right); //////////////////the rest of the BinOps follow this as well. 
  return pr ;
}

//! Expr ::= Expr forwardSlash Expr
ParseResult Parser::parseDivision ( ParseResult prLeft ) {
  // parser has already matched left expression
  ParseResult pr ;
  Expr *left = dynamic_cast<Expr *> (prLeft.ast);
  match ( forwardSlash ) ;
  string* op = new string(prevToken->lexeme);
  ParseResult prRight = parseExpr (prevToken->lbp());
  Expr *right = dynamic_cast<Expr *>(prRight.ast);
  pr.ast = new BinOpExpr(left,op,right);
  return pr ;
}


/*! Expr ::= Expr equalEquals Expr*/
/*! Expr ::= Expr lessThanEquals Expr*/
/*! Expr ::= Expr greaterThanEquals Expr*/
/*! Expr ::= Expr notEquals Expr*/
/*! Expr ::= Expr leftAngle Expr*/
/*! Expr ::= Expr rightAngle Expr*/

/*! Notice that for relational operators we use just one parse
   function.  This shows another possible means for implementing
   expressions, as opposed to the method used for arithmetic
   expressions in which each operation has its own parse method.  It
   will depend on what we do in iteration 3 in building an abstract
   syntax tree to decide which method is better.
 */
ParseResult Parser::parseRelationalExpr ( ParseResult prLeft ) {
    // parser has already matched left expression 
    ParseResult pr ;
    Expr *left = dynamic_cast<Expr *>(prLeft.ast);
    nextToken( ) ;
    // just advance token, since examining it in parseExpr caused
    // this method being called.
    string* op = new string(prevToken->lexeme) ;
    ParseResult prRight = parseExpr (currToken->lbp());
    
    Expr *right = dynamic_cast<Expr *>(prRight.ast);
    pr.ast = new BinOpExpr(left,op,right);
   
    return pr ;
}


//! Helper function used by the parser.

void Parser::match (tokenType tt) {
    if (! attemptMatch(tt)) {
        throw ( makeErrorMsgExpected ( tt ) ) ;
    }
}

//! Helper function used by the parser.
bool Parser::attemptMatch (tokenType tt) {
    if (currToken->terminal == tt) { 
        nextToken() ;
        return true ;
    }
    return false ;
}

//! Helper function used by the parser.
bool Parser::nextIs (tokenType tt) {
    return currToken->terminal == tt ;
}

//! Helper function used by the parser.
void Parser::nextToken () {
    if ( currToken == NULL ) 
        throw ( string("Internal Error: should not call nextToken in unitialized state"));
    else 
    if (currToken->terminal == endOfFile && currToken->next == NULL) {
        prevToken = currToken ;
    } else if (currToken->terminal != endOfFile && currToken->next == NULL) {
        throw ( makeErrorMsg ( "Error: tokens end with endOfFile" ) ) ;
    } else {
        prevToken = currToken ;
        currToken = currToken->next ;
    }
}

//! Helper function used by the parser.
string Parser::terminalDescription ( tokenType terminal ) {
    Token *dummyToken = new Token ("",terminal, NULL) ;
    ExtToken *dummyExtToken = extendToken (this, dummyToken) ;
    string s = dummyExtToken->description() ;
    delete dummyToken ;
    delete dummyExtToken ;
    return s ;
}

//! Helper function used by the parser.
string Parser::makeErrorMsgExpected ( tokenType terminal ) {
    string s = (string) "Expected " + terminalDescription (terminal) +
        " but found " + currToken->description() ;
    return s ;
}

//! Helper function used by the parser.
string Parser::makeErrorMsg ( tokenType terminal ) {
    string s = "Unexpected symbol " + terminalDescription (terminal) ;
    return s ;
}

//! Helper function used by the parser.
string Parser::makeErrorMsg ( const char *msg ) {
    return msg ;
}

