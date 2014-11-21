/*! \file ast.cpp
    \brief The abstract syntax tree .cpp file
    \author Sophia Stembridge
    \author Wen Chuan Lee
    
*/
#include "ast.h"

using namespace std ;

//Root
//----------------------------------------------

/*! \fn string Root::unparse()
    \brief Unparse for root node: varName '(' ')' '{' Stmts '}'
*/
string Root::unparse () {
    return varName->unparse() + " () {\n" + stmts->unparse() + "\n}\n" ;
}
string Root::cppCode(){
	return (string)
	"#include <iostream>\n" +
	"#include \"Matrix.h\"\n" +
	"#include <math.h>\n" +
	"using namespace std; \n" +
	"int main () { \n" +
	stmts->cppCode() +
	"\n}\n";
}

Root::~Root() {}

//Decl
//----------------------------------------------

/*! \fn string SimpleDecl::unparse()
    \brief Unparse for SimpleDecl node: integerKwd|floatKwd|stringKwd varName ';'
*/
string SimpleDecl::unparse(){
       return kwd + " " + var->unparse() + "; \n";
}

string SimpleDecl::cppCode(){
	return (string) "";
}

/*! \fn string MatrixDecl::unparse()
    \brief Unparse for MatrixDecl node : 'Matrix' varName '=' Expr ';'
*/
string MatrixDecl::unparse(){
       return "Matrix " + var1->unparse() + " = " + expr1->unparse() + ";  \n";
}

string MatrixDecl::cppCode(){
	return (string) "";
}

/*! \fn string LongMatrixDecl::unparse()
    \brief Unparse for LongMatrixDecl node : 'Matrix' varName '[' Expr ',' Expr ']' varName ',' varName  '=' Expr ';'
*/
string LongMatrixDecl::unparse(){
       return "Matrix " + var1->unparse() + " [" + expr1->unparse() + " , " + expr2->unparse() + "] " + var2->unparse() + " , " + var3->unparse() + " = " + expr3->unparse() + "; \n";
}

string LongMatrixDecl::cppCode(){
	return (string) "";
}

//Expr
//----------------------------------------------

/*! \fn string BinOpExpr::unparse()
    \brief Unparse for BinOpExpr node : Expr 'op' Expr
    ops are: * / + - > >= < <= == != && ||
*/
string BinOpExpr::unparse ( ) {
    //return "bin op";
	return  left->unparse() + " " + *op + " " + right->unparse() ;
}

string BinOpExpr::cppCode(){
	return (string) "";
}

/*! \fn string VarName::unparse()
    \brief Unparse for VarName node : varName
*/
string VarName::unparse ( ) { return lexeme ; } ;

string VarName::cppCode(){
	return (string) "";
}

/*! \fn string AnyConst::unparse()
    \brief Unparse for AnyConst node : integerConst | floatConst |  stringConst
*/
string AnyConst::unparse ( ) { return constString + " "; } ;

string AnyConst::cppCode(){
	return (string) "";
}

/*! \fn string MatrixRefExpr::unparse()
    \brief Unparse for MatrixRefExpr node : varName '[' Expr ',' Expr ']'
*/
string MatrixRefExpr::unparse(){
       return var->unparse() + " [" + expr1->unparse() + " , " + expr2->unparse() + " ]";
}

string MatrixRefExpr::cppCode(){
	return (string) "";
}

/*! \fn string NestOrFuncExpr::unparse()
    \brief Unparse for NestOrFuncExpr node : varName '(' Expr ')'
*/
string NestOrFuncExpr::unparse() {
       return var->unparse() + " (" + expr->unparse() + ")";
}

string NestOrFuncExpr::cppCode(){
	return (string) "";
}

/*! \fn string ParenExpr::unparse()
    \brief Unparse for ParenExpr node : '(' Expr ')'
*/
string ParenExpr::unparse() {
       return "(" + expr->unparse() + ")";
}

string ParenExpr::cppCode(){
	return (string) "";
}

/*! \fn string LetExpr::unparse()
    \brief Unparse for LetExpr node : 'let' Stmts 'in' Expr 'end'
*/
string LetExpr::unparse() {
       return "let " + stmts->unparse() + " in " + expr->unparse() + " end ";
}

string LetExpr::cppCode(){
	return (string) "";
}

/*! \fn string IfElseExpr::unparse()
    \brief Unparse for IfElseExpr node : 'if' Expr 'then' Expr 'else' Expr
*/
string IfElseExpr::unparse() {
       return "if " + expr1->unparse() + " then " + expr2->unparse() + " else " + expr3->unparse();
}

string IfElseExpr::cppCode(){
	return (string) "";
}

/*! \fn string NotExpr::unparse()
    \brief Unparse for NotExpr node : '!' Expr
*/
string NotExpr::unparse() {
       return "!" + expr->unparse();
}

string NotExpr::cppCode(){
	return (string) "";
}

// Stmts
// -----------------------------------------------------------

/*! \fn string EmptyStmts::unparse()
    \brief Unparse for EmptyStmts node : <<empty>>
*/
string EmptyStmts::unparse() {
  return " ";
}

string EmptyStmts::cppCode(){
	return (string) "";
}

/*! \fn string StmtsSeq::unparse()
    \brief Unparse for StmtsSeq node : Stmt Stmts
*/
string StmtsSeq::unparse() {
  return stmt->unparse() + stmts->unparse();
}

string StmtsSeq::cppCode(){
	return (string) "";
}

// Stmt
// -----------------------------------------------------------

/*! \fn string DeclStmt::unparse()
    \brief Unparse for DeclStmt node : Decl
*/
string DeclStmt::unparse() {
  return decl->unparse() ;
}

string DeclStmt::cppCode(){
	return (string) "";
}

/*! \fn string IfStmt::unparse()
    \brief Unparse for IfStmt node : 'if' '(' Expr ')' Stmt
*/
string IfStmt::unparse() {
  return "if (" + ifExpr->unparse() + ")" + thenStmt->unparse();
}

string IfStmt::cppCode(){
	return (string) "";
}

/*! \fn string IfElseStmt::unparse()
    \brief Unparse for IfElseStmt node : 'if' '(' Expr ')' Stmt 'else' Stmt
*/
string IfElseStmt::unparse() {
  return "if (" + ifExpr->unparse() + ")" + thenStmt->unparse() + "\n" + "else " + elseStmt->unparse();
}

string IfElseStmt::cppCode(){
	return (string) "";
}

/*! \fn string BlockStmt::unparse()
    \brief Unparse for BlockStmt node : '{' Stmts '}'
*/
string BlockStmt::unparse() {
  return "{ \n" + statements->unparse()+ "\n}";
}

string BlockStmt::cppCode(){
	return (string) "";
}

/*! \fn string PrintStmt::unparse()
    \brief Unparse for PrintStmt node : 'print' '(' Expr ')' ';'
*/
string PrintStmt::unparse() {
  return "print (" + printExpr->unparse() + " );" ;
}

string PrintStmt::cppCode(){
	return (string) "";
}

/*! \fn string AssignStmt::unparse()
    \brief Unparse for AssignStmt node : varName '=' Expr ';'
*/
string AssignStmt::unparse() {
  return var->unparse() + " = " + rightExpr-> unparse() + ";";
}

string AssignStmt::cppCode(){
	return (string) "";
}

/*! \fn string LongAssignStmt::unparse()
    \brief Unparse for LongAssignStmt node : varName '[' Expr ',' Expr ']' '=' Expr ';'	
*/
string LongAssignStmt::unparse() {
  return var->unparse() + "["+ leftExpr1->unparse() + "," + leftExpr2->unparse() +"] = " + rightExpr->unparse() +";";
}

string LongAssignStmt::cppCode(){
	return (string) "";
}

/*! \fn string WhileStmt::unparse()
    \brief Unparse for WhileStmt node : 'while' '(' Expr ')' Stmt
*/
string WhileStmt::unparse() { 
  return "while (" + whileExpr->unparse() + " )" + whileStmt->unparse();
}

string WhileStmt::cppCode(){
	return (string) "";
}

/*! \fn string ForStmt::unparse()
    \brief Unparse for ForStmt node : 'for' '(' varName '=' Expr ':' Expr ')' Stmt
*/
string ForStmt::unparse() {
  return "for (" + var->unparse() +" = " + expr1->unparse() + ":" + expr2->unparse() + ")" + statements->unparse();
}

string ForStmt::cppCode(){
	return (string) "";
}
 
