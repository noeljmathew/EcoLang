#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "codegen.h"
#include "dump.h"
#include "energy.h"
#include "ir.h"
#include "lexer.h"
#include "optimizer.h"
#include "parser.h"
#include "report.h"
#include "semantic.h"

using namespace std;

int main(int argc, char** argv) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <path-to-.c-file>\n";
        return 1;
    }

    ifstream file(argv[1]);
    if (!file) {
        cerr << "Could not open " << argv[1] << '\n';
        return 1;
    }

    stringstream buffer;
    buffer << file.rdbuf();

    try {
        Lexer lexer(buffer.str());
        vector<Token> tokens = lexer.scanTokens();

        Parser parser(tokens);
        unique_ptr<Program> program = parser.parse();

        // Real token-stream and AST output, straight off the lexer's
        // already-produced tokens and the parser's already-produced tree -
        // no new parsing/analysis logic, same as every other dump in this
        // file. Computed here (right after lexing/parsing succeed) but
        // printed below in the same batched cout block as everything else,
        // so - like every other section - they only ever appear on a fully
        // successful compile, never a partial one.
        string tokenDump = dumpTokens(tokens);
        string astDump = dumpAst(*program);

        SemanticAnalyzer analyzer;
        analyzer.analyze(*program);

        IRGenerator irGenerator;
        IRProgram ir = irGenerator.generate(*program);

        // Energy analysis and code generation must both see the
        // OPTIMIZED IR, not the raw IR that came straight out of the
        // IR generator.
        Optimizer optimizer;
        IRProgram optimizedIr = optimizer.optimize(ir);
        OptimizationStats optimizationStats = optimizer.getStats();

        EnergyReport energyReport = analyzeEnergy(optimizedIr);

        // analyzeEnergy() accepts any IRProgram (see its doc comment in
        // energy.h) - calling it a second time on the raw, pre-optimization
        // IR (already computed above for [IR Before Optimization]) costs
        // nothing beyond the call itself and needs no changes to
        // energy.h/energy.cpp. This is what makes a real, apples-to-apples
        // before/after energy comparison possible: both totals come from
        // the exact same hardware-weighted cost model, just applied to IR
        // before and after the optimizer ran.
        EnergyReport energyReportBefore = analyzeEnergy(ir);

        CodeGenerator codeGenerator;
        string rawTargetCode = codeGenerator.generate(ir);
        string targetCode = codeGenerator.generate(optimizedIr);

        ReportGenerator reportGenerator;
        string energyReportText = reportGenerator.generate(energyReport);
        string energyReportBeforeText = reportGenerator.generate(energyReportBefore);

        cout << "========================================\n";
        cout << "           EcoLang Compiler\n";
        cout << "========================================\n\n";

        cout << "[Tokens]\n";
        cout << "-----------------\n";
        cout << tokenDump << '\n';

        cout << "[AST]\n";
        cout << "-----------------\n";
        cout << astDump << '\n';

        cout << "[Optimization]\n";
        cout << "-----------------\n";
        cout << left << setw(27) << "Original IR Instructions"
             << ": " << optimizationStats.originalInstructionCount << '\n';
        cout << left << setw(27) << "Optimized IR Instructions"
             << ": " << optimizationStats.optimizedInstructionCount << '\n';
        cout << left << setw(27) << "Instructions Removed"
             << ": " << optimizationStats.instructionsRemoved << '\n';
        cout << '\n';
        cout << left << setw(27) << "Constant Folding"
             << ": " << optimizationStats.constantFoldingCount << '\n';
        cout << left << setw(27) << "Constant Propagation"
             << ": " << optimizationStats.constantPropagationCount << '\n';
        cout << left << setw(27) << "Algebraic Simplification"
             << ": " << optimizationStats.algebraicSimplificationCount << '\n';
        cout << left << setw(27) << "Dead Temporary Elimination"
             << ": " << optimizationStats.deadTemporaryEliminationCount << '\n';
        cout << '\n';

        cout << "[IR Before Optimization]\n";
        cout << "-----------------\n";
        cout << rawTargetCode << '\n';

        cout << "[Code Generation]\n";
        cout << "-----------------\n";
        cout << targetCode << '\n';

        cout << "[Energy Report Before Optimization]\n";
        cout << "----------------\n";
        cout << energyReportBeforeText;
        cout << '\n';

        cout << "[Energy Report]\n";
        cout << "----------------\n";
        cout << energyReportText;
    } catch (const runtime_error& error) {
        cerr << error.what() << '\n';
        return 1;
    }

    return 0;
}
