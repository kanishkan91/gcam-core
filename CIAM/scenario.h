/* scenario.h										*
 * This header contains the definition for	the 	*
 * the scenario class.								*
 * SHK  3/12/02										*/

#ifndef _SCENARIO_H_
#define _SCENARIO_H_
#pragma once

#include <xercesc/dom/DOM.hpp>
#include <string>
#include <iostream>

using namespace std;
using namespace xercesc;

// Scenario class
class Scenario
{
private:
	string name; //! Scenario name.
	string scenarioSummary;

public:
	Scenario(); // default construtor
	void clear();
	void XMLParse( const DOMNode* node );
	void toXML( ostream& out ) const;
	void toDebugXMLOpen( const int period, ostream& out ) const;
	void toDebugXMLClose( const int period, ostream& out ) const;
	string getName() const; // return scenario name
	string XMLCreateDate( const time_t& time ) const;
};

#endif // _SCENARIO_H_