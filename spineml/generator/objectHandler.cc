#include "objectHandler.h"

// pugixml includes
#include "pugixml/pugixml.hpp"

//------------------------------------------------------------------------
// SpineMLGenerator::ObjectHandler::Condition
//------------------------------------------------------------------------
void SpineMLGenerator::ObjectHandler::Condition::onObject(const pugi::xml_node &node,
                                                          unsigned int currentRegimeID, unsigned int targetRegimeID)
{
    // Get triggering code
    auto triggerCode = node.child("Trigger").child("MathInline");
    if(!triggerCode) {
        throw std::runtime_error("No trigger condition for transition between regimes");
    }

    // Expand out any aliases
    std::string triggerCodeString = triggerCode.text().get();
    expandAliases(triggerCodeString, m_Aliases);

    // Write trigger condition
    m_CodeStream << "if(" << triggerCodeString << ")" << CodeStream::OB(2);

    // Loop through state assignements
    for(auto stateAssign : node.children("StateAssignment")) {
        // Expand out any aliases in code string
        std::string stateAssignCodeString = stateAssign.child_value("MathInline");
        expandAliases(stateAssignCodeString, m_Aliases);

        // Write state assignement
        m_CodeStream << stateAssign.attribute("variable").value() << " = " << stateAssignCodeString << ";" << std::endl;
    }

    // If this condition results in a regime change
    if(currentRegimeID != targetRegimeID) {
        m_CodeStream << "_regimeID = " << targetRegimeID << ";" << std::endl;
    }

    // End of trigger condition
    m_CodeStream << CodeStream::CB(2);
}

//------------------------------------------------------------------------
// SpineMLGenerator::ObjectHandler::TimeDerivative
//------------------------------------------------------------------------
void SpineMLGenerator::ObjectHandler::TimeDerivative::onObject(const pugi::xml_node &node,
                                                               unsigned int, unsigned int)
{
    // Expand out any aliases in code string
    std::string codeString = node.child_value("MathInline");
    expandAliases(codeString, m_Aliases);

    // **TODO** identify cases where Euler is REALLY stupid
    m_CodeStream << node.attribute("variable").value() << " += DT * (" << codeString << ");" << std::endl;
}