/* ghg_mrk.cpp										*
 * This header contains the methods for the		*
 * Greenhouse Gas class.						*
 *       										*
 * SHK  12/11/00								*/

#include "Definitions.h"
#include <cassert>

// xml headers
#include "xmlHelper.h"
#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOM.hpp>

//** Other Headers ********
#include <iostream>
#include <string>
#include "modeltime.h" // model start, end, timestep and period info
#include "ghg_mrk.h"
#include "market.h"
#include "Marketplace.h"

using namespace std; // enables elimination of std::

extern Modeltime modeltime;
extern Marketplace marketplace;

//! Default construtor.
ghg_mrk::ghg_mrk(){
}

//! Clear member variables.
void ghg_mrk::clear(){
	name = "";
	unit = "";
	market = "";
	constraint.clear();
	emission.clear();
}

//! Create GHG markets
void ghg_mrk::setMarket( const string& regionName )
{
	// marketplace is a global object
	// name is GHG name
	marketplace.setMarket( regionName, market, name, Market::GHG );
        marketplace.setMarketToSolve (name, market);
	/* no need to use market.setPriceVector here unless GHG markets need
        initial prices read-in for the base year */  
}


//! Initializes data members from XML.
void ghg_mrk::XMLParse( const DOMNode* node ){
	
	DOMNodeList* nodeList;
	DOMNodeList* childNodeList;
	DOMNode* curr = 0;
	DOMNode* currChild = 0;
	string nodeName;
	string childNodeName;
	
	// PRECONDITION
	// assume we are passed a valid node.
	assert( node );

	// get the name attribute.
	name = XMLHelper<string>::getAttrString( node, "name" );
	
	#if( _DEBUG )
		cout << "\t Greenhouse gas market name set as " << name << endl;
	#endif
	
	// get all child nodes.
	nodeList = node->getChildNodes();

	// loop through the child nodes.
	for( int i = 0; i < nodeList->getLength(); i++ ){
		curr = nodeList->item( i );
		nodeName = XMLString::transcode( curr->getNodeName() );
		
		if( nodeName == "market" ){
			market = XMLHelper<string>::getValueString( curr ); // should be only one market
		}

		else if( nodeName == "period" ){
			childNodeList = curr->getChildNodes();
			
			// loop through the periods children.
			for( int j = 0; j < childNodeList->getLength(); j++ ){
				currChild = childNodeList->item( j );
				childNodeName = XMLString::transcode( currChild->getNodeName() );

				if( childNodeName == "constraint" ){
					constraint.push_back( XMLHelper<double>::getValue( currChild ) ); // only one constraint per period.
				}
			}
		}
	}
	// completed parsing.
	// not a read in value
	emission.resize( modeltime.getmaxper() ); // emissions (tgC or MTC)
}

//! Writes datamembers to datastream in XML format.
void ghg_mrk::toXML( ostream& out ) const {
	
	// write the beginning tag.
	Tabs::writeTabs( out );
	out << "<ghgmarket name=\"" << name << "\">" << endl;
	
	// increase the indent.
	Tabs::increaseIndent();

	// write the xml for the class members.
	XMLWriteElement( unit, "unit", out );
	// write out the market string.
	XMLWriteElement( market, "market", out );
	
	for( vector<double>::const_iterator i = constraint.begin(); i != constraint.end(); i++ ){
		Tabs::increaseIndent();
		Tabs::writeTabs( out );
		
		out << "<period>" << endl;
		
		Tabs::increaseIndent();
		XMLWriteElement( *i, "constraint", out );
		Tabs::decreaseIndent();

		Tabs::writeTabs( out );
		out << "</period>" << endl;	
		Tabs::decreaseIndent();
	}

	// finished writing xml for the class members.
	
	// decrease the indent.
	Tabs::decreaseIndent();
	
	// write the closing tag.
	Tabs::writeTabs( out );
	out << "</ghgmarket>" << endl;
}

//! Writes datamembers to datastream in XML format.
void ghg_mrk::toDebugXML( const int period, ostream& out ) const {
	
	// write the beginning tag.
	Tabs::writeTabs( out );
	out << "<ghgmarket name=\"" << name << "\">" << endl;
	
	// increase the indent.
	Tabs::increaseIndent();

	// Write the xml for the class members.
	XMLWriteElement( unit, "unit", out );

	// write out the market string.
	XMLWriteElement( market, "market", out );
	
	// Write the constraint for the current year
	XMLWriteElement( constraint[ period ], "constraint", out );

	XMLWriteElement( emission[ period ], "emission", out );
	// finished writing xml for the class members.
	
	// decrease the indent.
	Tabs::decreaseIndent();
	
	// write the closing tag.
	Tabs::writeTabs( out );
	out << "</ghgmarket>" << endl;
}

//! Set emissions.
void ghg_mrk::setEmission( const double amount, const int per )
{
	emission[ per ] = amount; // emissions (tgC or MTC)
}

//! Show emission name.
string ghg_mrk::getName() const
{
	return name;
}

//! Return emissions target.
double ghg_mrk::getConstraint( const int per ) const
{
	return constraint[ per ]; // emissions constraint (tgC or MTC)
}

//! Return emissions.
double ghg_mrk::getEmission( const int per ) const
{
	return emission[ per ]; // emissions (tgC or MTC)

}

