#include "query/Operator.h"

using namespace std;
using namespace boost;

namespace scidb
{

/**
 * @brief install_github.
 *
 * @par Synopsis:
 *   install_github( repo, branch )
 *
 * @par Summary:
 *   Download, build, and install a plugin from GitHub.
 *
 * @par Input:
 *   - repo: a repository formatted like user/repo, for example paradigm4/knn.
 *   - branch: optional repository branch, defaults to 'master'.
 *
 * @par Output array:
 *        <
 *   <br>   success: bool
 *   <br> >
 *   <br> [
 *   <br>   i: start=end=0, chunk interval=1.
 *   <br> ]
 *
 * @par Examples:
 *   install_github('paradigm4/load_tools','master')
 *
 * @par Errors:
 *   n/a
 *
 */
class Logicalinstall_github: public LogicalOperator
{
public:
	Logicalinstall_github(const string& logicalName, const string& alias):
        LogicalOperator(logicalName, alias)
    {
    	ADD_PARAM_CONSTANT("string")
    	ADD_PARAM_CONSTANT("string")
    	_usage = "install_github('repository', 'branch')";
    }

    ArrayDesc inferSchema(vector<ArrayDesc> inputSchemas, shared_ptr<Query> query)
    {
        Attributes atts(1);
        atts[0] = AttributeDesc((AttributeID)0, "success",  TID_BOOL, 0, 0 );
        Dimensions dims(1);
        dims[0] = DimensionDesc("i", 0, 0, 0, 0, 1, 0);
        return ArrayDesc("", atts, dims);
    }

};

REGISTER_LOGICAL_OPERATOR_FACTORY(Logicalinstall_github, "install_github");

} //namespace
