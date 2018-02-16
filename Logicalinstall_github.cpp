#include "query/Operator.h"

using namespace std;
#ifndef CPP11
//using namespace boost;
#endif

namespace scidb
{

/**
 * @brief install_github.
 *
 * @par Synopsis:
 *   install_github( repo [, branch] [, options] )
 *
 * @par Summary:
 *   Download, build, and install a plugin from GitHub.
 *
 * @par Input:
 *   - repo: a repository formatted like user/repo, for example paradigm4/knn.
 *   - branch: optional repository branch, defaults to 'master'.
 *   - options: optional defines inserted before make, for example SCIDB=/nonstandard/scidb/location
 *
 * @par Output array:
 *        <  success: bool >[i=0:0,1,0]
 *
 * @par Examples:
 *   install_github('paradigm4/load_tools')
 *
 */
class Logicalinstall_github: public LogicalOperator
{
public:
	Logicalinstall_github(const string& logicalName, const string& alias):
        LogicalOperator(logicalName, alias)
    {
    	ADD_PARAM_CONSTANT("string")
        ADD_PARAM_VARIES()
    	_usage = "install_github('repository' [, 'branch'] [, 'options'])";
    }

    std::vector<shared_ptr<OperatorParamPlaceholder> > nextVaryParamPlaceholder(const std::vector< ArrayDesc> &schemas)
    {  
        std::vector<shared_ptr<OperatorParamPlaceholder> > res;
        res.push_back(END_OF_VARIES_PARAMS());
        switch (_parameters.size()) {
          case 0:
/* No optional arguments */
            break;
          case 1:
/* branch */
            res.push_back(PARAM_CONSTANT("string"));
            break;
          case 2:
/* makefile options */
            res.push_back(PARAM_CONSTANT("string"));
            break;
        }
        return res;
    }

    ArrayDesc inferSchema(vector<ArrayDesc> inputSchemas, shared_ptr<Query> query)
    {
        Attributes atts(1);
        atts[0] = AttributeDesc((AttributeID)0, "success",  TID_BOOL, 0, CompressorType::NONE );
        Dimensions dims(1);
        dims[0] = DimensionDesc("i", 0, 0, 0, 0, 1, 0);
        //#ifdef CPP11
        return ArrayDesc("", atts, dims, defaultPartitioning(), query->getDefaultArrayResidency());
        //#else
        //return ArrayDesc("", atts, dims);
        //#endif
    }

};


REGISTER_LOGICAL_OPERATOR_FACTORY(Logicalinstall_github, "install_github");

} //namespace
