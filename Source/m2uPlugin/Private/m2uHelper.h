#ifndef _M2UHELPER_H_
#define _M2UHELPER_H_

// disable assignment within conditional expression error in VS
#ifdef _MSC_VER
#pragma warning( disable : 4706 )
#endif

#include "AssetSelection.h"
#include "m2uAssetHelper.h"
#include "Runtime/Launch/Resources/Version.h"

// Functions I'm currently using from this cpp file aren't exported, so they will
// be unresolved symbols on Windows. For now importing the cpp is OK...
// TODO: the required functions should probably be rewritten here instead.
#include "Editor/UnrealEd/Private/ParamParser.cpp"


// Provides functions that are used by most likely more than one command or action
namespace m2uHelper
{

	const FString M2U_GENERATED_NAME(TEXT("m2uGeneratedName"));


/**
   Parse a python-style list from a string to an array containing the contents
   of that list.
   The input string should look like this:
   [name1,name2,name3,name4]
 */
	TArray<FString> ParseList(FString Str)
	{
		FString Chopped = Str.Mid(1,Str.Len()-2); // remove the brackets
		TArray<FString> Result;
		Chopped.ParseIntoArray( Result, TEXT(","), false);
		return Result;
	}

/**
   tries to find an Actor by name and makes sure it is valid.
   @param Name The name to look for
   @param OutActor This will be the found Actor or NULL
   @param InWorld The world in which to search for the Actor

   @return true if found and valid, false otherwise
   TODO: narrow searching to the InWorld or current world if not set.
 */
bool GetActorByName( const TCHAR* Name, AActor** OutActor, UWorld* InWorld = NULL)
{
	if( InWorld == NULL)
	{
		InWorld = GEditor->GetEditorWorldContext().World();
	}
	AActor* Actor;
	Actor = FindObject<AActor>( InWorld->GetCurrentLevel(), Name, false );
	//Actor = FindObject<AActor>( ANY_PACKAGE, Name, false );
	// TODO: check if StaticFindObject or StaticFindObjectFastInternal is better
	// and if searching in current world gives a perfo boost, if thats possible
	if( Actor == NULL ) // actor with that name cannot be found
	{
		return false;
	}
	else if( ! Actor->IsValidLowLevel() )
	{
		//UE_LOG(LogM2U, Log, TEXT("Actor is NOT valid"));
		return false;
	}
	else
	{
		//UE_LOG(LogM2U, Log, TEXT("Actor is valid"));
		*OutActor=Actor;
		return true;
	}
}


/**
 * FName GetFreeName(const FString& Name)
 *
 * Will find a free (unused) name based on the Name string provided.
 * This will be achieved by increasing or adding a number-suffix until the
 * name is unique.
 *
 * @param Name A name string with or without a number suffix on which to build onto.
 */
	FName GetFreeName(const FString& Name)
	{
		// Generate a valid FName from the String

		FString GeneratedName = Name;
		// create valid object name from the string. (remove invalid characters)
		for( int32 BadCharacterIndex = 0; BadCharacterIndex < ARRAY_COUNT(
				 INVALID_OBJECTNAME_CHARACTERS ) - 1; ++BadCharacterIndex )
		{
			const TCHAR TestChar[2] = { INVALID_OBJECTNAME_CHARACTERS[
					BadCharacterIndex ], 0 };
			const int32 NumReplacedChars = GeneratedName.ReplaceInline( TestChar,
																		TEXT( "" ) );
		}

		FName TestName( *GeneratedName );
		if( TestName == NAME_None )
		{
			TestName = FName( *M2U_GENERATED_NAME );
		}

		// TODO: maybe check only inside the current level or so?
		//UObject* Outer = ANY_PACKAGE;
		UObject* Outer = GEditor->GetEditorWorldContext().World()->GetCurrentLevel();
		UObject* ExistingObject;

		// increase the suffix until there is no ExistingObject found
		for(;;)
		{
			if (Outer == ANY_PACKAGE)
			{
				ExistingObject = StaticFindObject( NULL, ANY_PACKAGE,
												   *TestName.ToString() );
			}
			else
			{
				ExistingObject = StaticFindObjectFast( NULL, Outer, 
															   TestName );
			}
			
			if( ! ExistingObject ) // current name is not in use
				break;
			// increase suffix
			//TestName = FName(TestName.GetIndex(), TestName.GetNumber() + 1 );
			TestName.SetNumber( TestName.GetNumber() + 1 );
		}
		return TestName;

	}// FName GetFreeName()

	
/**
 * void SetActorTransformRelativeFromText(AActor* Actor, const TCHAR* Stream)
 *
 * Set the Actors relative transformations to the values provided in text-form
 * T=(x y z) R=(x y z) S=(x y z)
 * If one or more of T, R or S is not present in the String, they will be ignored.
 *
 * Relative transformations are the actual transformation values you see in the 
 * Editor. They are equivalent to object-space transforms in maya for example.
 *
 * Setting world-space transforms using SetActorLocation or so will yield fucked
 * up results when using nested transforms (parenting actors).
 *
 * The Actor has to be valid, so check before calling this function!
 */
	void SetActorTransformRelativeFromText(AActor* Actor, const TCHAR* Str)
	{
		const TCHAR* Stream; // used for searching in Str
		//if( Stream != NULL )
		//{	++Stream;  } // skip a space

		// get location
		FVector Loc;
		if( (Stream =  FCString::Strfind(Str,TEXT("T="))) )
		{
			Stream += 3; // skip "T=("
			Stream = GetFVECTORSpaceDelimited( Stream, Loc );
			//UE_LOG(LogM2U, Log, TEXT("Loc %s"), *(Loc.ToString()) );
			Actor->SetActorRelativeLocation( Loc, false );
		}

		// get rotation
		FRotator Rot;
		if( (Stream =  FCString::Strfind(Str,TEXT("R="))) )
		{
			Stream += 3; // skip "R=("
			Stream = GetFROTATORSpaceDelimited( Stream, Rot, 1.0f );
			//UE_LOG(LogM2U, Log, TEXT("Rot %s"), *(Rot.ToString()) );
			Actor->SetActorRelativeRotation( Rot, false );
		}

		// get scale
		FVector Scale;
		if( (Stream =  FCString::Strfind(Str,TEXT("S="))) )
		{
			Stream += 3; // skip "S=("
			Stream = GetFVECTORSpaceDelimited( Stream, Scale );
			//UE_LOG(LogM2U, Log, TEXT("Scc %s"), *(Scale.ToString()) );
			Actor->SetActorRelativeScale3D( Scale );
		}

		Actor->InvalidateLightingCache();
		// Call PostEditMove to update components, etc.
		Actor->PostEditMove( true );
		Actor->CheckDefaultSubobjects();
		// Request saves/refreshes.
		Actor->MarkPackageDirty();

	}// void SetActorTransformRelativeFromText()




} // namespace m2uHelper
#endif /* _M2UHELPER_H_ */
