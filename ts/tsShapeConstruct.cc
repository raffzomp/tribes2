//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "ts/tsShapeConstruct.h"
#include "console/consoleTypes.h"
#include "Core/fileStream.h"
#include "Core/bitStream.h"

IMPLEMENT_CO_DATABLOCK_V1(TSShapeConstructor);

const char * TSShapeConstructor::csmShapeDirectory = "shapes/";

S32 gNumTransforms = 0;
S32 gRotTransforms = 0;
S32 gTransTransforms = 0;

TSShapeConstructor::TSShapeConstructor()
{
   mShape = NULL;
   for (S32 i = 0; i<MaxSequences; i++)
      mSequence[i] = NULL;
}

TSShapeConstructor::~TSShapeConstructor()
{
}

bool TSShapeConstructor::onAdd()
{
   if(!Parent::onAdd())
      return false;
      
   char fileBuf[256];
   dSprintf(fileBuf, sizeof(fileBuf), "%s%s", csmShapeDirectory, mShape);

   hShape = ResourceManager->load(fileBuf);
   if(!bool(hShape))
      return false;

   if (hShape->getSequencesConstructed())
      return true;
   
   // we want to write into the resource:
   TSShape * shape = const_cast<TSShape*>((const TSShape*)hShape);

   Stream * f;
   for (S32 i=0; i<MaxSequences; i++)
   {
      if (mSequence[i])
      {
//          // if there was a space or tab in mSequence[i], then a new name was supplied for it
//          // change it here...will still be in fileBuf
//          dSprintf(fileBuf, sizeof(fileBuf), "%s%s", csmShapeDirectory, mSequence[i]);
//          const char * terminate1a = dStrchr(fileBuf,' ');
//          const char * terminate2a = dStrchr(fileBuf,'\t');
//          if (terminate1a || terminate2a)
//          {
//             // select the latter one:
//             const char * nameStart = terminate1a<terminate2a ? terminate2a : terminate1a;
//             do
//                nameStart++;
//             while ( (*nameStart==' ' || *nameStart=='\t') && *nameStart != '\0' );
//             if (shape->findSequence(nameStart))
//                continue;
//          }


         // look for sequences in same directory as shapes...
         dSprintf(fileBuf, sizeof(fileBuf), "%s%s", csmShapeDirectory, mSequence[i]);
         // spaces and tabs indicate the end of the file name:
         char * terminate1 = dStrchr(fileBuf,' ');
         char * terminate2 = dStrchr(fileBuf,'\t');
         if (terminate1 && terminate2)
            // select the earliest one:
            *(terminate1<terminate2 ? terminate1 : terminate2) = '\0';
         else if (terminate1 || terminate2)
            // select the non-null one:
            *(terminate1 ? terminate1 : terminate2) = '\0';

         f = ResourceManager->openStream(fileBuf);
         if (!f || f->getStatus() != Stream::Ok)
         {
            Con::errorf(ConsoleLogEntry::General,"Missing sequence %s for %s",mSequence[i],mShape);
            continue;
         }
         if (!shape->importSequences(f) || f->getStatus()!=Stream::Ok)
         {
            Con::errorf(ConsoleLogEntry::General,"Load sequence %s failed for %s",mSequence[i],mShape);
            return false;
         }
         ResourceManager->closeStream(f);
         
         // if there was a space or tab in mSequence[i], then a new name was supplied for it
         // change it here...will still be in fileBuf
         if (terminate1 || terminate2)
         {
            // select the latter one:
            char * nameStart = terminate1<terminate2 ? terminate2 : terminate1;
            do
               nameStart++;
            while ( (*nameStart==' ' || *nameStart=='\t') && *nameStart != '\0' );
            // find the name in the shape (but keep the old one there in case used by something else)
            shape->sequences.last().nameIndex = shape->findName(nameStart);
            if (shape->sequences.last().nameIndex == -1)
            {
               shape->sequences.last().nameIndex = shape->names.size();
               shape->names.increment();
               shape->names.last() = StringTable->insert(nameStart,false);
            }
         }
      }
      else
         break;
   }

   hShape->setSequencesConstructed(true);
   return true;
}

void TSShapeConstructor::packData(BitStream* stream)
{
   Parent::packData(stream);
   stream->writeString(mShape);

   S32 count = 0;
   for (S32 b=0; b<MaxSequences; b++)
      if (mSequence[b])
         count++;
   stream->writeInt(count,NumSequenceBits);

   for (S32 i=0; i<MaxSequences; i++)
      if (mSequence[i])
         stream->writeString(mSequence[i]);
}

void TSShapeConstructor::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);
   mShape = stream->readSTString();
   
   S32 i = 0, count = stream->readInt(NumSequenceBits);
   for (; i<count; i++)
      mSequence[i] = stream->readSTString();
   while (i<MaxSequences)
      mSequence[i++] = NULL;
}

void TSShapeConstructor::consoleInit()
{
}

void TSShapeConstructor::initPersistFields()
{
   Parent::initPersistFields();
   addField("baseShape", TypeString, Offset(mShape, TSShapeConstructor));
   char buf[30];
   for (S32 i=0; i<MaxSequences; i++)
   {
      dSprintf(buf,sizeof(buf),"sequence%i",i);
      addField(buf, TypeString, Offset(mSequence[i], TSShapeConstructor));
   }
}


