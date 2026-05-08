#pragma once

typedef signed char int8;
typedef short int16;
typedef int int32;
typedef long long int64;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned long uint32;
typedef unsigned long long uint64;


enum class EClassCastFlags : uint64
{
	None = 0x0000000000000000,

	Field = 0x0000000000000001,
	Int8Property = 0x0000000000000002,
	Enum = 0x0000000000000004,
	Struct = 0x0000000000000008,
	ScriptStruct = 0x0000000000000010,
	Class = 0x0000000000000020,
	ByteProperty = 0x0000000000000040,
	IntProperty = 0x0000000000000080,
	FloatProperty = 0x0000000000000100,
	UInt64Property = 0x0000000000000200,
	ClassProperty = 0x0000000000000400,
	UInt32Property = 0x0000000000000800,
	InterfaceProperty = 0x0000000000001000,
	NameProperty = 0x0000000000002000,
	StrProperty = 0x0000000000004000,
	Property = 0x0000000000008000,
	ObjectProperty = 0x0000000000010000,
	BoolProperty = 0x0000000000020000,
	UInt16Property = 0x0000000000040000,
	Function = 0x0000000000080000,
	StructProperty = 0x0000000000100000,
	ArrayProperty = 0x0000000000200000,
	Int64Property = 0x0000000000400000,
	DelegateProperty = 0x0000000000800000,
	NumericProperty = 0x0000000001000000,
	MulticastDelegateProperty = 0x0000000002000000,
	ObjectPropertyBase = 0x0000000004000000,
	WeakObjectProperty = 0x0000000008000000,
	LazyObjectProperty = 0x0000000010000000,
	SoftObjectProperty = 0x0000000020000000,
	TextProperty = 0x0000000040000000,
	Int16Property = 0x0000000080000000,
	DoubleProperty = 0x0000000100000000,
	SoftClassProperty = 0x0000000200000000,
	Package = 0x0000000400000000,
	Level = 0x0000000800000000,
	Actor = 0x0000001000000000,
	PlayerController = 0x0000002000000000,
	Pawn = 0x0000004000000000,
	SceneComponent = 0x0000008000000000,
	PrimitiveComponent = 0x0000010000000000,
	SkinnedMeshComponent = 0x0000020000000000,
	SkeletalMeshComponent = 0x0000040000000000,
	Blueprint = 0x0000080000000000,
	DelegateFunction = 0x0000100000000000,
	StaticMeshComponent = 0x0000200000000000,
	MapProperty = 0x0000400000000000,
	SetProperty = 0x0000800000000000,
	EnumProperty = 0x0001000000000000,
	SparseDelegateFunction = 0x0002000000000000,
	MulticastInlineDelegateProperty = 0x0004000000000000,
	MulticastSparseDelegateProperty = 0x0008000000000000,
	FieldPathProperty = 0x0010000000000000,
	// Removed							= 0x0200000000000000,
	// Removed							= 0x0400000000000000,
	LargeWorldCoordinatesRealProperty = 0x0080000000000000,
	OptionalProperty = 0x0100000000000000,
	VValueProperty = 0x0200000000000000,
	VerseVMClass = 0x0400000000000000,
	VRestValueProperty = 0x0800000000000000,
	Utf8StrProperty = 0x1000000000000000,
	AnsiStrProperty = 0x2000000000000000,
	VCellProperty = 0x4000000000000000,
};

namespace Offsets
{

	constexpr uint64 GNames = 0x4AAB6C0;
	constexpr uint64 GWorld = 0x4C2F3B0;
	constexpr uint64 GObjects = 0x4AE7A10;

	namespace UObject
	{
		constexpr uint64 Class = 0x10; //The type of Class is UClass

		constexpr uint64 SuperStruct = 0x40;
		constexpr uint64 Name = 0x18; // Through this offset you can find the object's Id, which can be used with GetName to find the object's name
		constexpr uint64 Size = 0x18; // This needs adjustment based on the actual situation. Size is the element size in GObjects array. Use it to find the next object's address.

		// Through this offset you can find the object's OuterPrivate, which is an object itself.
		// By using its Id and GetName function, you can find the name of OuterPrivate.
		// Its OuterPrivate is also an object, and you can repeat this process
		// until OuterPrivate is 0. This forms the complete path-qualified object name (e.g. "Engine.World.PersistentLevel.MyActor")
		constexpr uint64 Outer = 0x20;
		constexpr uint64 ChildProperties = 0x50; // The type of ChildProperties is FField, which is a linked list. +0x20 points to the next data member, +0x28 is the FFieldClass type address, and +0x40 contains data member properties.
		namespace FField
		{
			constexpr uint64 ClassPrivate = 0x8; // The type of ClassPrivate is FFieldClass
			namespace FFieldClass
			{
				constexpr uint64 CastFlags = 0x10;
			}

			constexpr uint64 Next = 0x20;
			constexpr uint64 Name = 0x28;
			constexpr uint64 EnumProperty = 0x78;
			constexpr uint64 BoolOrUint8Mask = 0x78;


			constexpr uint64 ElementSize = 0x3C;
			constexpr uint64 Offset_Internal = 0x4C;
		}
	}
}