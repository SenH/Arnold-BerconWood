#pragma once

#include "c4dtoa_api.h"
#include "c4dtoa_symbols.h"

#include "c4d.h"

// unique id obtained from http://www.plugincafe.com/forum/developer.asp 
// NOTE add this id to the mtd file of your shader: 
//    c4d.node_id            INT     {id} 
#define BERCONWOOD_ID {id}

class ArnoldBErconWood : public ArnoldShaderGvOperatorData
{
public:

   ///
   /// Constructor.
   ///
   static NodeData* Alloc()
   {
      return NewObjClear(ArnoldBErconWood);
   }

   ///
   /// Node initialization function.
   ///
   virtual void InitValues(GeListNode* node);

   ///
   /// Defines the Arnold node entry which belongs to this node.
   ///
   virtual String GetAiNodeEntryName(BaseList2D* node);

   ///
   /// Event handler
   ///
   virtual Bool Message(GeListNode* node, Int32 type, void* data);
};

static Bool RegisterArnoldBErconWood()
{
   return GvRegisterOperatorPlugin(BERCONWOOD_ID, "BerconWood", GV_OPERATORFLAG_NONE, ArnoldBErconWood::Alloc, "ainode_BerconWood", 0,
      ID_GV_OPCLASS_TYPE_GENERAL, ID_GV_OPGROUP_TYPE_GENERAL, ARNOLD_RENDERER, 0);
}

