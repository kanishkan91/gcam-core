/*
 * LEGAL NOTICE
 * This computer software was prepared by Battelle Memorial Institute,
 * hereinafter the Contractor, under Contract No. DE-AC05-76RL0 1830
 * with the Department of Energy (DOE). NEITHER THE GOVERNMENT NOR THE
 * CONTRACTOR MAKES ANY WARRANTY, EXPRESS OR IMPLIED, OR ASSUMES ANY
 * LIABILITY FOR THE USE OF THIS SOFTWARE. This notice including this
 * sentence must appear on any copies of this computer software.
 * 
 * EXPORT CONTROL
 * User agrees that the Software will not be shipped, transferred or
 * exported into any country or used in any manner prohibited by the
 * United States Export Administration Act or any other applicable
 * export laws, restrictions or regulations (collectively the "Export Laws").
 * Export of the Software may require some form of license or other
 * authority from the U.S. Government, and failure to obtain such
 * export control license may result in criminal liability under
 * U.S. laws. In addition, if the Software is identified as export controlled
 * items under the Export Laws, User represents and warrants that User
 * is not a citizen, or otherwise located within, an embargoed nation
 * (including without limitation Iran, Syria, Sudan, Cuba, and North Korea)
 *     and that User is not otherwise prohibited
 * under the Export Laws from receiving the Software.
 * 
 * All rights to use the Software are granted on condition that such
 * rights are forfeited if User fails to comply with the terms of
 * this Agreement.
 * 
 * User agrees to identify, defend and hold harmless BATTELLE,
 * its officers, agents and employees from all liability involving
 * the violation of such Export Laws, either directly or indirectly,
 * by User.
 */

/*! 
* \file sector_report.cpp
* \ingroup Objects
* \brief The SectorReport class source file.
* 
* \author Praelit Patel
* \author Josh Lurz
*/

#include "util/base/include/definitions.h"
#include <string>

#include "reporting/include/sector_report.h"
#include "reporting/include/storage_table.h"

#include "sectors/include/sector.h"
#include "technologies/include/production_technology.h"
#include "functions/include/iinput.h"
#include "util/base/include/model_time.h"
#include "containers/include/scenario.h" // for modeltime
#include "util/base/include/util.h"
#include "containers/include/region.h"

extern Scenario* scenario; // for modeltime.

using namespace std;

//! Default Constructor
SectorReport::SectorReport( ostream& aFile ):
mFile( aFile ),
mTable( new StorageTable )
{
}

/*! \brief For outputing SGM data to a flat csv File
 * 
 * \author Sonny Kim
 */
void SectorReport::finish() const {
    if ( !mTable->isEmpty() ){ // only print out for Production Sectors
        mFile << "-----------------------------" << endl;
        mFile << "Sector Report for Production Sector:   " << mSectorName << endl;
        mFile << "-----------------------------" << endl;
        mFile << "Vintages" ;
        mFile << " ";
        // write out column headings (years)
        // Get the list of row names.
        const vector<string> rowNames = mTable->getRowLabels();
        
        // Get the column labels
        const vector<string> colNames = mTable->getColLabels();
        for( vector<string>::const_iterator col = colNames.begin(); col != colNames.end(); ++col ){
            mFile << ',' << *col;
        }   
        mFile << endl;

        // Note: This is structurally different from SAM. This goes through the rows and prints
        // out each of the category values.
        for ( vector<string>::const_iterator row = rowNames.begin(); row != rowNames.end(); ++row ){
            mFile << *row;
            // Iterate through the columns.
            for( vector<string>::const_iterator col = colNames.begin(); col != colNames.end(); ++col ) {
                mFile << ',' << mTable->getValue( *row, *col );
            }
            mFile << endl;
        }
        mFile << endl;
    }
}

void SectorReport::startVisitRegion( const Region* aRegion, const int aPeriod ){
    // Cache the region name.
    mCurrRegion = aRegion->getName();
}

void SectorReport::endVisitRegion( const Region* aRegion, const int aPeriod ){
    // Clear the cached region name.
    mCurrRegion.clear();
}

void SectorReport::startVisitSector( const Sector* sector, const int aPeriod ) {
    mSectorName = sector->getName();
}

//! Update the SectorReport with information from the the ProductionTechnology.
// This function is currently hacked to get ordering right.
void SectorReport::startVisitProductionTechnology( const ProductionTechnology* prodTechnology,
                                               const int aPeriod )
{
    if( aPeriod == -1 || ( prodTechnology->isAvailable( aPeriod ) &&
            !prodTechnology->isRetired( aPeriod ) ) ) {
        // Create a unique name for the vintage based on the name and year.
        const string currName = util::toString( prodTechnology->getYear() ) + prodTechnology->getName();
        
        // Add a column for the current tech.
        mTable->addColumn( currName );

        // Add the values for the technology.
        if( prodTechnology->isNewInvestment( aPeriod ) ){
            mTable->addToType( "Annual Investment", currName, prodTechnology->mAnnualInvestment );
            mTable->addToType( "Capital Stock", currName, prodTechnology->capital );
        }
        mTable->addToType( "Output", currName, prodTechnology->getOutput( aPeriod ) );
        mTable->addToType( "Profits", currName, prodTechnology->mProfits[ aPeriod ] );
        mTable->addToType( "Costs", currName, prodTechnology->mCostsReporting[ aPeriod ] );
        // This isn't right, retained earnings has values by period.
        mTable->addToType( "Retained Earnings", currName, prodTechnology->expenditures[ aPeriod ].getValue(Expenditure::RETAINED_EARNINGS) );
        if( prodTechnology->isNewInvestment( aPeriod ) ){
            mTable->setType( "Expected Profit Rate", currName, prodTechnology->mExpectedProfitRateReporting );
            // mTable->setType( "Expected Price Received", currName, prodTechnology->mExpectedPriceReceivedReporting );
        }
        const Modeltime* modeltime = scenario->getModeltime();
        // Its questionable if this should be here or in updateInput.
        for( unsigned int i = 0; i < prodTechnology->input.size(); ++i ){
            // report price paid only for new vintage.
            // Since the new vintage is the last vintage, this also allows price paid to be the last column.
            if( prodTechnology->isNewInvestment( aPeriod ) || 
                // If we are in the base year the base year technology is the newest technology.
                // There is no new investment.
                ( aPeriod == modeltime->getBasePeriod() && 
                modeltime->getper_to_yr( modeltime->getBasePeriod() ) == prodTechnology->year ) )
            {
                // Add a column to the table.
                mTable->addColumn( "Price Paid" );
                mTable->setType( prodTechnology->input[ i ]->getName(), "Price Paid",
                    prodTechnology->input[ i ]->getPricePaid( mCurrRegion, aPeriod ) );
            }
            mTable->addToType( prodTechnology->input[ i ]->getName(), currName,
                prodTechnology->input[ i ]->getCurrencyDemand( aPeriod ) );
        }
    }
}

