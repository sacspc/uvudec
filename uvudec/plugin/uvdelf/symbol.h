/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_ELF_SYMBOL_H
#define UVD_ELF_SYMBOL_H

#include "uvd/data/data.h"
#include "uvdelf/header.h"
#include "uvd/util/types.h"
#include "uvd/relocation/relocation.h"
#include <elf.h>

/*
Some sort of globally visible symbol
All symbols occur in a single table and each symbol has a reference to the relavent section
There will be an assumption for now that since all relocations I want are in functions,
all relocations will be done in symbols' data

This is a hybrid between UVDRelocatableElement and UVDRelocatableData, 
but don't like multiple inheritance
UVDRelocatableData is instead put inside
*/
class UVDElfRelocation;
class UVDElfSymbolSectionHeaderEntry;
class UVDElfSymbol : public UVDRelocatableElement
{
public:
	UVDElfSymbol();
	~UVDElfSymbol();
	
	virtual uv_err_t init();

	//Superseeded by UVDRelocatableElement::getName(), setName()
	//Set symbol name
	//void setSymbolName(const std::string &sName);
	//Get symbol name
	//std::string getSymbolName();

	//Symbol payload
	uv_err_t getData(UVDData **data);
	uv_err_t setData(UVDData *data);
	
	uv_err_t addRelocation(UVDElfRelocation *relocation);
	
	//Get a template relocation value for this symbol
	uv_err_t getRelocation(UVDElfRelocation **relocationOut);
	
	/*
	From TIS ELF specification

	STB_LOCAL Local symbols are not visible outside the object file containing their
	definition. Local symbols of the same name may exist in multiple files
	without interfering with each other.
	STB_GLOBAL Global symbols are visible to all object files being combined. One file's
	definition of a global symbol will satisfy another file's undefined reference
	to the same global symbol.
	STB_WEAK Weak symbols resemble global symbols, but their definitions have lower
	precedence.
	STB_LOPROC through Values in this inclusive range are reserved for processor-specific semantics.
	STB_HIPROC
	In each symbol table,
	*/
	//Deprecated for setBinding(), more tech correct in ELF speak
	//void setVisibility(uint32_t visibility);
	//uv_err_t getVisibility(uint32_t *visibility);
 	
 	uv_err_t setBinding(uint32_t binding);
 	uv_err_t getBinding(uint32_t *binding);
 	uv_err_t setType(uint32_t type);
 	uv_err_t getType(uint32_t *type);
 	
 	//Get the header entry with the fixups necessary to push it to the file
 	uv_err_t getHeaderEntryRelocatable(UVDRelocatableData **symbolEntryRelocatable);
	
	//Update symbol string table
	virtual uv_err_t updateForWrite();
	//virtual uv_err_t constructForWrite();
	//Get string table refs
	virtual uv_err_t applyRelocationsForWrite();

  	//The symbol's (function's/variable's) name

protected:
	//virtual uv_err_t addSymbolNameRelocation();
	//The symbol is about to be saved and all its relocatables should be updated
	//virtual uv_err_t updateRelocations();
	//Every symbol type has specific values depending on defined data
	virtual uv_err_t updateType() = 0;

public:
	//The symbol's (function's/variable's) name
	//std::string m_name;
	//The actual data this symbol represents, if its resolved
	//If the symbol is not resolved, data will be NULL
	//Relocatable form so we can do some util stuff like get zerod version
	UVDRelocatableData m_relocatableData;
	//For the header entry
	UVDRelocatableData m_headerEntryRelocatableData;
	//The ELF symbol structure
	Elf32_Sym m_symbol;
	//Relocations
	//std::vector<UVDElfRelocation *> m_relocations;

	//The section header this belongs to
	UVDElfSymbolSectionHeaderEntry *m_symbolSectionHeader;

	//The section header this relocation applies to, if its not special
	//ie if STT_SECTION is used, used to compute st_shndx
	UVDElfSectionHeaderEntry *m_relevantSectionHeader;
	
};

/*
A global variable
*/
class UVDElfVariableSymbol : public UVDElfSymbol
{
public:
	UVDElfVariableSymbol();
	~UVDElfVariableSymbol();

protected:
	uv_err_t updateType();

public:
};

/*
A peice of binary data identified as a function
For convienence only 
*/
class UVDElfFunctionSymbol : public UVDElfSymbol
{
public:
	UVDElfFunctionSymbol();
	~UVDElfFunctionSymbol();

	virtual uv_err_t updateForWrite();

protected:
	uv_err_t updateType();

public:
	//Binary function data
	//UVDData *m_data;
	//Position in the file written out, not in memory
	//Note this is not used to relocate data during ELF file writting, but rather to generate relocation table
	//UVDRelocatableData *m_fileRelocatableData;
};

/*
Section link symbol
These are used to specify relocations relative to the start of a section
*/
class UVDElfSectionSymbol : public UVDElfSymbol
{
public:
	UVDElfSectionSymbol();
	~UVDElfSectionSymbol();

	virtual uv_err_t updateForWrite();

protected:
	uv_err_t updateType();

public:
};

/*
File name symbol
*/
class UVDElfFilenameSymbol : public UVDElfSymbol
{
public:
	UVDElfFilenameSymbol();
	~UVDElfFilenameSymbol();

	virtual uv_err_t updateForWrite();
	//virtual uv_err_t applyRelocationsForWrite();

protected:
	uv_err_t updateType();

public:
};

/*
Null symbol
*/
class UVDElfNullSymbol : public UVDElfSymbol
{
public:
	UVDElfNullSymbol();
	~UVDElfNullSymbol();
	static uv_err_t getUVDElfNullSymbol(UVDElfNullSymbol **symbolOut);

protected:
	uv_err_t updateType();
	//virtual uv_err_t addSymbolNameRelocation();

public:
};

/*
Section that holds actual symbol data
*/
class UVDElfSymbolSectionHeaderEntry : public UVDElfSectionHeaderEntry
{
public:
	UVDElfSymbolSectionHeaderEntry();
	~UVDElfSymbolSectionHeaderEntry();

	virtual uv_err_t init();
	virtual uv_err_t initRelocatableData();

	uv_err_t addSymbol(UVDElfSymbol *symbol);
	//Add before iter
	//Useful for certain symbols such as the null or file symbol that need specific order
	uv_err_t addSymbolCore(UVDElfSymbol *symbol, const std::vector<UVDElfSymbol *>::iterator &iter);

	uv_err_t findSymbol(const std::string &sName, UVDElfSymbol **symbol);
	uv_err_t prepareSymbol(UVDElfSymbol *symbol);
	uv_err_t prepareSymbolCore(UVDElfSymbol *symbol, uint32_t shouldAdd);
	//A typical symbol having a name and such
	//It will need to be further refined as a function etc
	//Whether or not the symbol is defined will be determined by set data 
	uv_err_t getFunctionSymbol(const std::string &name, UVDElfSymbol **symbolOut);
	uv_err_t getVariableSymbol(const std::string &name, UVDElfSymbol **symbolOut);
	uv_err_t getFilenameSymbol(const std::string &name, UVDElfSymbol **symbolOut);
	//Add a STT_SECTION entry
	uv_err_t addSectionSymbol(const std::string &section);
	
	//Used during writting relocations to ELF file
	uv_err_t getSymbolIndex(const UVDElfSymbol *symbool, uint32_t *index);
	//How far into the specified section, in bytes, the symbol definition is
	//only valid for defined symbols
	uv_err_t getSymbolSectionOffset(UVDElfSymbol *symbol, uint32_t *offsetOut);
	
	//The symbol table has string relocations, non-trivial to return
	//virtual uv_err_t updateDataCore();
	//virtual uv_err_t syncDataAfterUpdate();

	//uv_err_t getSymbolStringRelocatableElement(const std::string &s, UVDRelocatableElement **relocatable);
	uv_err_t getSymbolStringIndex(const std::string &s, uint32_t *index);
		
	virtual uv_err_t updateForWrite();
	virtual uv_err_t constructForWrite();
	virtual uv_err_t applyRelocationsForWrite();
	
	//The filename this object will be attributed to being sourced from
	uv_err_t setSourceFilename(const std::string &file);

protected:
	uv_err_t getSectionSymbol(const std::string &section, UVDElfSymbol **symbol);
	uv_err_t addNullSymbol();
		
public:
	std::vector<UVDElfSymbol *> m_symbols;
	//Special symbols
	UVDElfSymbol *m_fileNameSymbol;
	//Do we need a symbol specifying the section symbols are in?
};

#endif
