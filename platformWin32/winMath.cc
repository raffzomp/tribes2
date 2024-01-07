//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "console/console.h"
#include "math/mMath.h"


extern void mInstallLibrary_C();
extern void mInstallLibrary_ASM();
extern void mInstall_AMD_Math();
extern void mInstall_Library_SSE();



//--------------------------------------
static void cMathInit(SimObject*, S32 argc, const char** argv)
{
   U32 properties = CPU_PROP_C;  // C entensions are always used
   
   if (argc == 1)
   {
         Math::init(0);
         return;
   }
   for (argc--, argv++; argc; argc--, argv++)
   {
      if (dStricmp(*argv, "DETECT") == 0) { 
         Math::init(0);
         return;
      }
      if (dStricmp(*argv, "C") == 0) { 
         properties |= CPU_PROP_C; 
         continue; 
      }
      if (dStricmp(*argv, "FPU") == 0) { 
         properties |= CPU_PROP_FPU; 
         continue; 
      }
      if (dStricmp(*argv, "MMX") == 0) { 
         properties |= CPU_PROP_MMX; 
         continue; 
      }
      if (dStricmp(*argv, "3DNOW") == 0) { 
         properties |= CPU_PROP_3DNOW; 
         continue; 
      }
      if (dStricmp(*argv, "SSE") == 0) { 
         properties |= CPU_PROP_SSE; 
         continue; 
      }
      Con::printf("Error: MathInit(): ignoring unknown math extension '%s'", *argv);
   }
   Math::init(properties);
}



//------------------------------------------------------------------------------
void Math::init(U32 properties)
{
   Con::addCommand( "MathInit",   cMathInit,   "MathInit(detect|C|FPU|MMX|3DNOW|SSE|...)",    1, 10);
   
   if (!properties)
      // detect what's available
      properties = Platform::SystemInfo.processor.properties;  
   else
      // Make sure we're not asking for anything that's not supported
      properties &= Platform::SystemInfo.processor.properties;  

   Con::printf("Math Init:");
   Con::printf("   Installing Standard C extensions");
   mInstallLibrary_C();

   Con::printf("   Installing Assembly extensions");
   mInstallLibrary_ASM();

   if (properties & CPU_PROP_FPU)
   {
      Con::printf("   Installing FPU extensions");
   }
   if (properties & CPU_PROP_MMX)
   {
      Con::printf("   Installing MMX extensions");
      if (properties & CPU_PROP_3DNOW)
      {
         Con::printf("   Installing 3DNow extensions");
         mInstall_AMD_Math();
      }
   }
   if (properties & CPU_PROP_SSE)
   {
      Con::printf("   Installing SSE extensions");
      mInstall_Library_SSE();
   }
   Con::printf(" ");
}   


