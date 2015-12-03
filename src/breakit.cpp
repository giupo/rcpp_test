
#include <Rcpp.h>
#include <R_ext/Parse.h>


/* string encoding handling */
#if (R_VERSION < R_Version(2,8,0)) || (defined DISABLE_ENCODING)
#define mkRChar(X) mkChar(X)
#else
#define USE_ENCODING 1
cetype_t string_encoding = CE_NATIVE;  /* default is native */
#define mkRChar(X) Rf_mkCharCE((X), string_encoding)
#endif


#if R_VERSION < R_Version(2,5,0)
#define RS_ParseVector R_ParseVector
#else
#define RS_ParseVector(A,B,C) R_ParseVector(A,B,C,R_NilValue)
#endif

using namespace Rcpp;

SEXP parseString(const char *s, int *parts, ParseStatus *status) {
  int maxParts = 1;
  const char *c = s;
  SEXP cv, pr = R_NilValue;
  
  while (*c) {
		if (*c == '\n' || *c == ';') maxParts++;
		c++;
  }
  
  PROTECT(cv = Rf_allocVector(STRSXP, 1));
  SET_STRING_ELT(cv, 0, mkRChar(s));  
  
  while (maxParts > 0) {
		pr = RS_ParseVector(cv, maxParts, status);
		if (*status != PARSE_INCOMPLETE && *status != PARSE_EOF) break;
		maxParts--;
  }
  UNPROTECT(1);
  *parts = maxParts;
  
  return pr;
}

class Parser {
public:
  Parser() {
  }

  SEXP evaluate(std::string& x, SEXP data) {
    // now construct the call to eval: 
    // and evaluate it
    ParseStatus status;
    int parts;
    SEXP parsedCMD = parseString(x.c_str(), &parts, &status);
    return Rf_eval(Rf_eval(parsedCMD, Environment::global_env() ),
                   Environment::global_env());
  }
};

RCPP_MODULE(parser){
  class_<Parser>("Parser")
    .constructor()
    .method("evaluate", &Parser::evaluate , "parse it")
    ;
}


