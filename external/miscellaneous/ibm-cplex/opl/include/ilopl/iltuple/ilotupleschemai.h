// -------------------------------------------------------------- -*- C++ -*-
// File: ./include/ilopl/iltuple/ilotupleschemai.h
// --------------------------------------------------------------------------
// Licensed Materials - Property of IBM
//
// 5725-A06 5725-A29 5724-Y48 5724-Y49 5724-Y54 5724-Y55
// Copyright IBM Corp. 2000, 2024
//
// US Government Users Restricted Rights - Use, duplication or
// disclosure restricted by GSA ADP Schedule Contract with
// IBM Corp.
// ---------------------------------------------------------------------------

#ifndef __ADVANCED_ilotupleschemaiH
#define __ADVANCED_ilotupleschemaiH

#ifdef _WIN32
#pragma pack(push, 8)
#endif
#include <ilopl/ilosys.h>

#include <ilopl/iltuple/ilodatacolumni.h>

class IloTupleSchemaI;
class IloTupleSchema;

class IloTuplePattern;

class IloColumnDefinitionI : public IloEnvObjectI{
  friend class IloTupleSchemaI; 
  IloColumnDefinitionI(IloColumnDefinitionI const &); 
  IloColumnDefinitionI &operator=(IloColumnDefinitionI const &); 
protected:
  
  
  
  
  
  
  
  
  
  IloDataCollection::IloDataType _type;
  IloSymbolI* _name;
  IloBool _isKey;
  IloAny _data;
  IloDataCollection::SortSense _sort;

  void setName(const char* name);
  
  IloColumnDefinitionI(IloEnv env, IloDataCollection::IloDataType type, const char* name = 0);
public:

  IloBool isOrdered() const { return _sort == IloDataCollection::ORDERED; }
  IloBool isSorted() const { return _sort == IloDataCollection::ASC; }
  IloBool isReversed() const { return _sort == IloDataCollection::DESC; }
  IloDataCollection::SortSense getSortSense() const { return _sort; }
  void setSortSense(IloDataCollection::SortSense sense) { _sort = sense; }
  
  IloDataCollection::IloDataType getDataType() const {
    return _type;
  }

  void setKeyProperty(IloBool type) { _isKey = type; }

  IloBool getKeyProperty() const { return _isKey; }

  
  void setData(IloAny data){
    _data = data;
  }
  
  IloAny getData() const {
    return _data;
  }

  
  const char* getName() const {
    return (_name ? _name->getString() : 0);
  }
  IloSymbolI* getSymbolName() const{
    return _name;
  }

  virtual ~IloColumnDefinitionI(){}

  
  IloBool isInt() const { return _type == IloDataCollection::IntDataColumn; }
  
  IloBool isNum() const { return _type == IloDataCollection::NumDataColumn; }
  
  IloBool isSymbol() const { return _type == IloDataCollection::SymbolDataColumn; }
  
  IloBool isAny() const { return _type == IloDataCollection::AnyDataColumn; }

  
  IloBool isIntCollection() const { return _type == IloDataCollection::IntCollectionColumn; }
  
  IloBool isNumCollection() const { return _type == IloDataCollection::NumCollectionColumn; }
  
  IloBool isAnyCollection() const { return _type == IloDataCollection::AnyCollectionColumn; }

  
  virtual IloBool isTuple() const;
  
  virtual void display(ILOSTD(ostream)& out) const;
};

class IloTupleBufferI;

class IloTupleSchemaI : public IloRttiEnvObjectI {
  ILORTTIDECL
private:
  IloBool _hasKey;                  
  typedef IloTupleSchemaI *SchemaPtr; 
  SchemaPtr mutable   _keySchema;   
  IloIntArray mutable _keyTotalIdx; 
  IloIntArray mutable _keyIdx;      
  IloBool _hasSubTuple; 
  IloSymbolI* _name;    
  typedef IloArray<IloColumnDefinitionI *> ColumnDefinitionArray;
  ColumnDefinitionArray         _array;      
  ColumnDefinitionArray mutable _totalArray; 
  IloIntArray2 mutable _sharedPaths;      
  IloIntArray mutable  _simpleColumnsIds; 
  IloIntArray mutable _intColsAbsIdx;    
  IloIntArray mutable _numColsAbsIdx;    
  IloIntArray mutable _symbolColsAbsIdx; 
  IloIntArray mutable _intColsKeyAbsKeyIdx;    
  IloIntArray mutable _numColsKeyAbsKeyIdx;    
  IloIntArray mutable _symbolColsKeyAbsKeyIdx; 
  typedef IloAny2IndexHashTable *HashPtr; 
  HashPtr mutable _hash;      
  HashPtr mutable _hashTotal; 
  IloIntArray _empty;
  IloIntArray mutable _collectionColumnIdx; 
  IloTupleCellArray mutable _emptyCells;
  typedef IloTupleBufferI *BufferPtr; 
  BufferPtr mutable _sharedBuffer;
public:
  IloDataCollection::IloDataType getColumnType(IloIntArray path) const;
private:
  void makeSharedPaths() const;
  
  IloIntArray makePathFromAbsolutePosition(IloInt position) const;
  void buildSharedIntColsAbsIdx() const;
  void buildSharedNumColsAbsIdx() const;
  void buildSharedSymbolColsAbsIdx() const;
  void getOrMakeHash() const;
  void getOrMakeTotalHash() const;
  void makeCollectionColumnIndexes() const;
  ColumnDefinitionArray const &getArray() const { return _array; }
  ColumnDefinitionArray const &makeTotalColumnDefinitionArray() const;
  ColumnDefinitionArray const &getOrMakeTotalColumnDefinitionArray() const {
    
    if (!_hasSubTuple)
      return getArray();
    
    
    if (_totalArray.getImpl())
      return _totalArray;

    return makeTotalColumnDefinitionArray();
  }

  
  IloColumnDefinitionI &getMutableColumn(IloInt i) const { return *_array[i]; }
  IloColumnDefinitionI &getMutableColumn(IloIntArray path) const;
  IloColumnDefinitionI &getMutableColumn(IloIntArray path, IloInt size) const;
  IloColumnDefinitionI &getMutableColumn(IloInt size, IloInt const *path) const;

public:
  IloTupleBufferI* getOrMakeSharedTupleBuffer() const;
  IloTupleCellArray getOrMakeEmptyCells() const;
  IloIntArray getCollectionColumnIndexes() const{ return _collectionColumnIdx;}
  IloIntArray getEmpty() const { return _empty; }
  void makeShared() const {
    if (_sharedPaths.getImpl()) return;
    makeCollectionColumnIndexes();
    makeSharedPaths();
    buildSharedIntColsAbsIdx();
    buildSharedNumColsAbsIdx();
    buildSharedSymbolColsAbsIdx();
  }
  IloIntArray getSharedIntColsAbsIdx() const{
    return _intColsAbsIdx;
  }
  IloIntArray getSharedNumColsAbsIdx() const{
    return _numColsAbsIdx;
  }
  IloIntArray getSharedSymbolColsAbsIdx() const{
    return _symbolColsAbsIdx;
  }
  IloIntArray getSharedIntColsKeyAbsKeyIdx() const{
    return _intColsKeyAbsKeyIdx;
  }
  IloIntArray getSharedNumColsKeyAbsKeyIdx() const{
    return _numColsKeyAbsKeyIdx;
  }
  IloIntArray getSharedSymbolColsKeyAbsKeyIdx() const{
    return _symbolColsKeyAbsKeyIdx;
  }

  IloIntArray getSharedPathFromAbsolutePosition(IloInt position) const {
    return _sharedPaths[position];
  }
  
  
  IloInt getInternalId(IloIntArray path) const;

  IloTuplePattern makeTuplePattern() const;
  IloBool hasSubTuple() const { return _hasSubTuple; }
  IloBool hasCollectionColumn() const { return _collectionColumnIdx.getImpl() != 0; }
  void setSubTuple(IloBool flag){ _hasSubTuple = flag; }
  void setName(const char* name);
  IloSymbolI* getSymbolName() const {
    return _name;
  }

  IloBool isCompatibleWith(IloTupleSchemaI const *) const;

  
  IloBool isSimpleTypedSchema() const;
  virtual ~IloTupleSchemaI();

  void setKeyProperty(const char* col);
  void setKeyProperty(IloSymbolI const *col);
  void setKeyProperty(IloInt idx);

  IloBool hasKeyProperty(const char* col) const;
  IloBool hasKeyProperty(IloSymbolI const *col) const;
  IloBool hasKeyProperty(IloInt idx) const;

  IloBool hasKey() const { return _hasKey; }
  void setKey(IloBool f) { _hasKey = f; }

  IloTupleSchemaI* getOrMakeSharedKeySchema() const;
  IloIntArray getOrMakeTotalKeyIndexes() const;
  IloIntArray getOrMakeKeyIndexes() const;

  IloInt getTotalIndexFromKey(IloInt key) const;
  IloInt getIndexFromKey(IloInt key) const;

  IloInt getIndexFromTotalIndex( IloInt totalIndex ) const;

  
  IloTupleSchemaI(IloEnv env, const char* name = 0);

  
  IloInt getSize() const{
    
    return _array.getImpl()==NULL ? 0 : _array.getSize();
  }

  
  IloInt getTotalSize() const {
    return getOrMakeTotalColumnDefinitionArray().getSize();
  }

  
  IloInt getTotalColumnNumber() const {
    if (!_hasSubTuple) return getArray().getSize();
    return getOrMakeTotalColumnDefinitionArray().getSize();
  }

  
  IloColumnDefinitionI const &getColumn(IloInt i) const { return getMutableColumn(i); }

  IloColumnDefinitionI const &getTotalColumn(IloInt i) const {
    return *getOrMakeTotalColumnDefinitionArray()[i];
  }

  IloInt getColumnIndex(const char* name) const;
  IloInt getColumnIndex(const IloSymbolI* name) const;

  IloInt getTotalColumnIndex(const char* name) const;
  IloInt getTotalColumnIndex(const IloSymbolI* name) const;

  const char* getColumnName(IloInt idx) const {
    return getColumn(idx).getName();
  }

  
  IloColumnDefinitionI const &getColumn(IloIntArray path) const { return getMutableColumn(path); }
  
  IloColumnDefinitionI const &getColumn(IloIntArray path, IloInt size) const { return getMutableColumn(path, size); }
  IloColumnDefinitionI const &getColumn(IloInt size, IloInt const *path) const { return getMutableColumn(size, path); }

  
  IloColumnDefinitionI &addIntColumn(const char* name =0);
  IloColumnDefinitionI &addNumColumn(const char* name =0);
  IloColumnDefinitionI &addAnyColumn(const char* name =0);
  IloColumnDefinitionI &addSymbolColumn(const char* name =0);
  IloColumnDefinitionI &addIntCollectionColumn(const char* name =0);
  IloColumnDefinitionI &addNumCollectionColumn(const char* name =0);
  IloColumnDefinitionI &addAnyCollectionColumn(const char* name =0);
  IloColumnDefinitionI &addTupleColumn(IloTupleSchemaI const *ax, const char* name=0);

  IloTupleSchemaI const *getTupleColumn(IloInt colIndex) const;
  IloTupleSchemaI const *getTupleColumn(IloIntArray path) const;
  IloTupleSchemaI const *getTupleColumn(IloInt size, IloInt* path) const;

  
  void clear();

  
  void display(ILOSTD(ostream)& out) const;

  IloBool isInt(IloInt index) const     { return _array[index]->isInt(); }
  IloBool isNum(IloInt index) const     { return _array[index]->isNum(); }
  IloBool isAny(IloInt index) const;
  IloBool isSymbol(IloInt index) const  { return _array[index]->isSymbol(); }
  IloBool isTuple(IloInt index) const   { return _array[index]->isTuple(); }
  IloBool isIntCollection(IloInt index) const { return _array[index]->isIntCollection(); }
  IloBool isNumCollection(IloInt index) const { return _array[index]->isNumCollection(); }
  IloBool isAnyCollection(IloInt index) const { return _array[index]->isAnyCollection(); }

  IloBool isInt(IloIntArray path) const     { return getColumnType(path) == IloDataCollection::IntDataColumn; }
  IloBool isNum(IloIntArray path) const     { return getColumnType(path) == IloDataCollection::NumDataColumn; }
  IloBool isAny(IloIntArray path) const;
  IloBool isSymbol(IloIntArray path) const  { return getColumnType(path) == IloDataCollection::SymbolDataColumn; }
  IloBool isTuple(IloIntArray path) const   { return getColumnType(path) == IloDataCollection::TupleSet; }
  IloBool isIntCollection(IloIntArray path) const { return getColumnType(path) == IloDataCollection::IntCollectionColumn; }
  IloBool isNumCollection(IloIntArray path) const { return getColumnType(path) == IloDataCollection::NumCollectionColumn; }
  IloBool isAnyCollection(IloIntArray path) const { return getColumnType(path) == IloDataCollection::AnyCollectionColumn; }

  
  void setData(IloInt index, IloAny data) { _array[index]->setData(data); }
  
  void setData(IloIntArray path, IloAny data);
  
  IloAny getData(IloInt index) const { return _array[index]->getData(); }
  
  IloAny getData(IloIntArray path) const;

  IloTupleSchemaI* copy() const;
  virtual IloBool isOplRefCounted() const ILO_OVERRIDE { return IloTrue; }
  IloBool isNamed() const;
  virtual IloRttiEnvObjectI* makeOplClone(IloEnvI* env) const ILO_OVERRIDE;

  static bool equals(IloTupleSchemaI const *s1, IloTupleSchemaI const *s2);
  bool operator==(IloTupleSchemaI const &other) const { return equals(this, &other); }
  bool operator!=(IloTupleSchemaI const &other) const { return !equals(this, &other); }
};

#ifdef _WIN32
#pragma pack(pop)
#endif

#endif

