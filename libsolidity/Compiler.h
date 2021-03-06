/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Solidity AST to EVM bytecode compiler.
 */

#include <ostream>
#include <libsolidity/ASTVisitor.h>
#include <libsolidity/CompilerContext.h>

namespace dev {
namespace solidity {

class Compiler: private ASTConstVisitor
{
public:
	explicit Compiler(bool _optimize = false): m_optimize(_optimize), m_returnTag(m_context.newTag()) {}

	void compileContract(ContractDefinition const& _contract, std::vector<MagicVariableDeclaration const*> const& _magicGlobals,
						 std::map<ContractDefinition const*, bytes const*> const& _contracts);
	bytes getAssembledBytecode() { return m_context.getAssembledBytecode(m_optimize); }
	void streamAssembly(std::ostream& _stream) const { m_context.streamAssembly(_stream); }

private:
	/// Registers the global objects and the non-function objects inside the contract with the context.
	void initializeContext(ContractDefinition const& _contract, std::vector<MagicVariableDeclaration const*> const& _magicGlobals,
						   std::map<ContractDefinition const*, bytes const*> const& _contracts);
	/// Adds the code that is run at creation time. Should be run after exchanging the run-time context
	/// with a new and initialized context.
	/// adds the constructor code.
	void packIntoContractCreator(ContractDefinition const& _contract, CompilerContext const& _runtimeContext);
	void appendConstructorCall(FunctionDefinition const& _constructor);
	/// Recursively searches the call graph and returns all functions needed by the constructor (including itself).
	std::set<FunctionDefinition const*> getFunctionsNeededByConstructor(FunctionDefinition const& _constructor);
	void appendFunctionSelector(ContractDefinition const& _contract);
	/// Creates code that unpacks the arguments for the given function, from memory if
	/// @a _fromMemory is true, otherwise from call data. @returns the size of the data in bytes.
	unsigned appendCalldataUnpacker(FunctionDefinition const& _function, bool _fromMemory = false);
	void appendReturnValuePacker(FunctionDefinition const& _function);

	void registerStateVariables(ContractDefinition const& _contract);

	virtual bool visit(FunctionDefinition const& _function) override;
	virtual bool visit(IfStatement const& _ifStatement) override;
	virtual bool visit(WhileStatement const& _whileStatement) override;
	virtual bool visit(ForStatement const& _forStatement) override;
	virtual bool visit(Continue const& _continue) override;
	virtual bool visit(Break const& _break) override;
	virtual bool visit(Return const& _return) override;
	virtual bool visit(VariableDefinition const& _variableDefinition) override;
	virtual bool visit(ExpressionStatement const& _expressionStatement) override;

	void compileExpression(Expression const& _expression);

	bool const m_optimize;
	CompilerContext m_context;
	std::vector<eth::AssemblyItem> m_breakTags; ///< tag to jump to for a "break" statement
	std::vector<eth::AssemblyItem> m_continueTags; ///< tag to jump to for a "continue" statement
	eth::AssemblyItem m_returnTag; ///< tag to jump to for a "return" statement
};

}
}
