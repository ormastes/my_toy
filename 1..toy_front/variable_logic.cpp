#include "variable_logic.h"

#include "llvm/Support/Error.h"
#include "common.h"
#include <string>
#include "ast.h"
class FunctionPrototypeAST;

void variable_bootup_init() {}

void variable_InitializeModule() {}
void variable_post_main() {}
std::unique_ptr<FunctionPrototypeAST> variable_parse_top_level(SourceLocation CurLoc) {
	return std::make_unique<FunctionPrototypeAST>(CurLoc, "main", std::vector<std::string>()) ;

}
