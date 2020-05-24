extern Stack* INPUT_STACK;
extern unsigned int getArity(Term* operation);
extern Hold* evaluateOperationTerm(Closure* operation, Closure* left, Closure* right);
