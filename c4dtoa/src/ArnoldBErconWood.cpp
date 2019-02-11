#include "ArnoldBErconWood.h"

#include "ainode_BerconWood.h"

void ArnoldBErconWood::InitValues(GeListNode* node)
{
   return ArnoldShaderGvOperatorData::InitValues(node);
}

String ArnoldBErconWood::GetAiNodeEntryName(BaseList2D* node)
{
   return String("BerconWood");
}

Bool ArnoldBErconWood::Message(GeListNode* node, Int32 type, void* data)
{
   return ArnoldShaderGvOperatorData::Message(node, type, data);
}

