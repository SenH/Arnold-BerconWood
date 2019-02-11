#include "ArnoldBErconWoodTranslator.h"

#include "ainode_BerconWood.h"

ArnoldBErconWoodTranslator::ArnoldBErconWoodTranslator(BaseList2D* node, RenderContext* context) : AbstractGvShaderTranslator(BERCONWOOD_TRANSLATOR, node, context)
{
}

char* ArnoldBErconWoodTranslator::GetAiNodeEntryName()
{
   return "BerconWood";
}

void ArnoldBErconWoodTranslator::InitSteps(int nsteps)
{
   // init all node array parameters
   AbstractGvShaderTranslator::InitSteps(nsteps);

   BaseList2D* node = (BaseList2D*)GetC4DNode();
   if (!m_aiNode || !node) return;
}

void ArnoldBErconWoodTranslator::Export(int step)
{
   // exports all node parameters
   AbstractGvShaderTranslator::Export(step);

   BaseList2D* node = (BaseList2D*)GetC4DNode();
   if (!m_aiNode || !node) return;

   // first motion step
   if (step == 0)
   {
   }
}

