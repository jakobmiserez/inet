import inet.common.INETDefs;
import inet.common.TagBase;

namespace inet;

class FlowLabelTagBase extends TagBase {
    uint32_t flowLabel = 0;
}

class FlowLabelReq extends FlowLabelTagBase {

}

class FlowLabelInd extends FlowLabelTagBase {
  
}