#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "codegen.h"
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

        CodeGenerator codeGenerator;
        string targetCode = codeGenerator.generate(optimizedIr);

        ReportGenerator reportGenerator;
        string energyReportText = reportGenerator.generate(energyReport);

        cout << "========================================\n";
        cout << "           EcoLang Compiler\n";
        cout << "========================================\n\n";

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

        cout << "[Code Generation]\n";
        cout << "-----------------\n";
        cout << targetCode << '\n';

        cout << "[Energy Report]\n";
        cout << "----------------\n";
        cout << energyReportText;
    } catch (const runtime_error& error) {
        cerr << error.what() << '\n';
        return 1;
    }

    return 0;
}
