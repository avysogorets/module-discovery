// -------------------------------------------------------------- -*- C++ -*-
// File: ./include/ilopl/ilooplsettings.h
// --------------------------------------------------------------------------
// Licensed Materials - Property of IBM
//
// 5725-A06 5725-A29 5724-Y48 5724-Y49 5724-Y54 5724-Y55 
// Copyright IBM Corp. 1998, 2024
//
// US Government Users Restricted Rights - Use, duplication or
// disclosure restricted by GSA ADP Schedule Contract with
// IBM Corp.
// ---------------------------------------------------------------------------

#ifndef __OPL_ilooplsettingsH
#define __OPL_ilooplsettingsH

#ifdef _WIN32
#pragma pack(push, 8)
#endif

#include <ilopl/ilooplsettingsi.h>

#include <ilopl/ilooplprofiler.h>
#include <ilopl/ilooplerrorhandler.h>

class IloOplModel;

class IloOplExecutionControllerBaseI;

class ILOOPL_EXPORTED IloOplExecutionController {
    HANDLE_DECL_OPL(IloOplExecutionController)
public:
    
    IloOplExecutionController(IloOplExecutionControllerBaseI* impl);

    
    std::ostream& getOut() const {
        return impl().getOut();
    }

    
    void enableAbort() {
        return impl().enableAbort(IloTrue);
    }

    
    void disableAbort() {
        return impl().enableAbort(IloFalse);
    }

    
    void abort() {
        return impl().abort();
    }

    
    void notifyNew(const IloOplModel opl);
    
    void notifyEnd(const IloOplModel opl);
    
    void notifyCall(const IloOplModel opl);
    
    void notifyReturn(const IloOplModel opl, IloBool status);
    
    void notifyNew(IloCplex algorithm);
    
    void notifyEnd(IloCplex algorithm) ;
    
    void notifyCall(IloCplex algorithm);
    
    void notifyReturn(IloCplex algorithm, IloBool status);
    
    void notifyNew(IloCP algorithm);
    
    void notifyEnd(IloCP algorithm) ;
    
    void notifyCall(IloCP algorithm);
    
    void notifyReturn(IloCP algorithm, IloBool status);
};

class ILOOPL_EXPORTED IloOplExecutionControllerBaseI: public IloOplExecutionControllerI {
    IloOplExecutionControllerI* _delegate;

public:
    
    IloOplExecutionControllerBaseI(IloEnv env, IloOplExecutionController deleg):IloOplExecutionControllerI(env.getImpl()), _delegate(&deleg.impl()) {
    }

protected:
    virtual std::ostream& getOut() const ILO_OVERRIDE;
    virtual IloBool overrideBlock(const char* name) ILO_OVERRIDE;
    virtual IloBool overrideMain(IloInt& status) ILO_OVERRIDE;

    virtual void enableAbort(IloBool enable) ILO_OVERRIDE;
    virtual void abort() ILO_OVERRIDE;

    virtual void notifyNew(const IloOplModelI& opl) ILO_OVERRIDE;
    virtual void notifyEnd(const IloOplModelI& opl) ILO_OVERRIDE;
    virtual void notifyCall(const IloOplModelI& opl) ILO_OVERRIDE;
    virtual void notifyReturn(const IloOplModelI& opl, IloBool status) ILO_OVERRIDE;

    virtual void notifyNew(IloAlgorithmI& algorithm) ILO_OVERRIDE;
    virtual void notifyEnd(IloAlgorithmI& algorithm) ILO_OVERRIDE;
    virtual void notifyCall(IloAlgorithmI& algorithm) ILO_OVERRIDE;
    virtual void notifyReturn(IloAlgorithmI& algorithm, IloBool status) ILO_OVERRIDE;

public:
   
    virtual ~IloOplExecutionControllerBaseI() {} 

   
    IloOplExecutionController getDelegate() const {
        return _delegate;
    }

    
    virtual void notifyNew(IloOplModel opl) =0;
    
    virtual void notifyEnd(IloOplModel opl) =0;
    
    virtual void notifyCall(IloOplModel opl) =0;
    
    virtual void notifyReturn(IloOplModel opl, IloBool status) =0;
    
    virtual void notifyNew(IloCplex cplex) =0;
    
    virtual void notifyEnd(IloCplex cplex) =0;
    
    virtual void notifyCall(IloCplex cplex) =0;
    
    virtual void notifyReturn(IloCplex cplex, IloBool status) =0;
    
    virtual void notifyNew(IloCP cp) =0;
    
    virtual void notifyEnd(IloCP cp) =0;
    
    virtual void notifyCall(IloCP cp) =0;
    
    virtual void notifyReturn(IloCP cp, IloBool status) =0;
};

class IloOplResourceResolverBaseI;

class ILOOPL_EXPORTED IloOplResourceResolver {
    HANDLE_DECL_OPL(IloOplResourceResolver)
public:
    
    IloOplResourceResolver(IloOplResourceResolverBaseI* impl);
};

class IloOplModelSource;
class IloOplDataSource;
class IloOplModel;

class ILOOPL_EXPORTED IloOplResourceResolverBaseI: public IloOplResourceResolverI {
public:
    
    IloOplResourceResolverBaseI(IloEnv env):IloOplResourceResolverI(env.getImpl()) {
    }

public:
   
    virtual ~IloOplResourceResolverBaseI() {} 

    
    virtual std::istream* doResolveStream(const char* basePath, const char* name) ILO_OVERRIDE;

   
    IloOplModelSource resolveModelSource(const char* basePath, const char* name);

   
    IloOplDataSource resolveDataSource(const char* basePath, const char* name);

   
    std::istream* resolveStream(const char* basePath, const char* name);

   
    const char* resolvePath(const char* basePath, const char* name) ILO_OVERRIDE;
};

class IloOplRunSettings {
private:
    IloOplSettingsI* _impl;

    const IloOplSettingsI& impl() const { ASSERT_IMPL; return *_impl; } \
    IloOplSettingsI& impl() { ASSERT_IMPL; return *_impl; } \

public:
    IloOplRunSettings(IloOplSettingsI* impl):_impl(impl) {
    }
    IloOplRunSettings():_impl(0) {
    }

    
    IloInt getMaxErrors() const {
        return impl().getRunMaxErrors();
    }
    
    void setMaxErrors(IloInt max) {
        impl().setRunMaxErrors(max);
    }

    
    IloInt getMaxWarnings() const {
        return impl().getRunMaxWarnings();
    }
    
    void setRunMaxWarnings(IloInt max) {
        impl().setRunMaxWarnings(max);
    }

    
    IloBool isProcessFeasibleSolutions() const {
        return impl().isRunProcessFeasibleSolutions();
    }
    
    void setProcessFeasibleSolutions(IloBool flag) {
        impl().setRunProcessFeasibleSolutions(flag);
    }

    
    IloBool isOaaSProcessFeasibleSolutions() const {
        return impl().isOaaSProcessFeasibleSolutions();
    }
    
    void setOaaSProcessFeasibleSolutions(IloBool flag) {
        impl().setOaaSProcessFeasibleSolutions(flag);
    }

    
    IloBool isDisplaySolution() const {
        return impl().isRunDisplaySolution();
    }
    
    void setDisplaySolution(IloBool flag) {
        impl().setRunDisplaySolution(flag);
    }

    
    IloBool isDisplayRelaxations() const {
        return impl().isRunDisplayRelaxations();
    }
    
    void setDisplayRelaxations(IloBool flag) {
        impl().setRunDisplayRelaxations(flag);
    }

    
    IloBool isDisplayConflicts() const {
        return impl().isRunDisplayConflicts();
    }
    
    void setDisplayConflicts(IloBool flag) {
        impl().setRunDisplayConflicts(flag);
    }

    
    IloBool isDisplayProfile() const {
        return impl().isRunDisplayProfile();
    }
    
    void setDisplayProfile(IloBool flag) {
        impl().setRunDisplayProfile(flag);
    }

    
    const char* getEngineLog() const {
        return impl().getRunEngineLog();
    }
    
    void setEngineLog(const char* name) {
        impl().setRunEngineLog(name);
    }

    
    IloBool isCallPopulate() const {
        return impl().isRunCallPopulate();
    }
    
    void setCallPopulate(IloBool flag) {
        impl().setRunCallPopulate(flag);
    }

    
    const char* getEngineExportExtension() const {
        return impl().getRunEngineExportExtension();
    }
    
    void setEngineExportExtension(const char* name) {
        impl().setRunEngineExportExtension(name);
    }
};

class IloOplSettings {
    HANDLE_DECL_BASE_WITHOUTEND_OPL(IloOplSettings,IloOplSettingsI,_impl)
protected:
    ImplClass* _impl;
public:
   
    IloOplSettings(IloEnv env, IloOplErrorHandler handler):_impl(0) {
        _impl = new (env) IloOplSettingsI(env.getImpl(),handler.impl());
        _impl->incrementRefCount();
    }

    
    IloOplSettings(IloEnv env, IloOplErrorHandler handler, IljVirtualMachine& vm):_impl(0) {
        _impl = new (env) IloOplSettingsI(env.getImpl(),handler.impl(),vm);
        _impl->incrementRefCount();
    }

    
    void end() {
        impl().decrementRefCount();
        _impl=0;
    }

    
    IljVirtualMachine* getVM() const {
        return impl().getVM();
    }

    
     void setExecutionController(IloOplExecutionController controller) {
        impl().setExecutionController(controller.impl());
    }

     
     void removeExecutionController(IloOplExecutionController controller) {
         impl().removeExecutionController(controller.impl());
     }

     
    IloOplExecutionController getExecutionController() const {
        return &impl().getExecutionController();
    }

    
    void setWithLocations(IloBool with) {
        impl().setWithLocations(with);
    }

    
    IloBool isWithLocations() const {
        return impl().isWithLocations();
    }

    
    void setWithNames(IloBool with) {
        impl().setWithNames(with);
    }

    
    IloBool isWithNames() const {
        return impl().isWithNames();
    }

    
    void setSkipAssert(IloBool with) {
        impl().setSkipAssert(with);
    }

    
    IloBool isSkipAssert() const {
        return impl().isSkipAssert();
    }

    
    IloBool isExportInternalData() const{
        return impl().isExportInternalData();
	}
    
	const char* getExportInternalData() const{
        return impl().getExportInternalData();
	}
    
	void setExportInternalData(const char* path){
        impl().setExportInternalData(path);
	}

    
    IloBool isExportExternalData() const{
        return impl().isExportExternalData();
	}
    
	const char* getExportExternalData() const{
        return impl().getExportExternalData();
	}
    
	void setExportExternalData(const char* path){
        impl().setExportExternalData(path);
	}

    
    void setWithWarnings(IloBool with) {
        impl().setWithWarnings(with);
    }

    
    IloBool isWithWarnings() const {
        return impl().isWithWarnings();
    }

    void setWithDebugInfo(IloBool with) {
        impl().setWithDebugInfo(with);
    }

    IloBool isWithDebugInfo() const {
        return impl().isWithDebugInfo();
    }

    void setCloudMode(IloBool with) {
        impl().setCloudMode(with);
    }

    IloBool isCloudMode() const {
        return impl().isCloudMode();
    }

    
	void setWithDataChecks(IloBool with) {
        impl().setWithDataChecks(with);
    }

    
	IloBool isWithDataChecks() const {
        return impl().isWithDataChecks();
    }

    
    void setForceElementUsage(IloBool onoff) {
        impl().setForceElementUsage(onoff);
    }

    
    IloBool isForceElementUsage() const {
        return impl().isForceElementUsage();
    }

	
	void setForceElementPostProcessingUsage(IloBool onoff) {
		impl().setForceElementPostProcessingUsage(onoff);
	}

	
	IloBool isForceElemenPostProcessingtUsage() const {
		return impl().isForceElementPostProcessingUsage();
	}

    
    void setSkipWarnNeverUsedElements(IloBool with) {
        impl().setSkipWarnNeverUsedElements(with);
    }

    
    IloBool isSkipWarnNeverUsedElements() const {
        return impl().isSkipWarnNeverUsedElements();
    }

    
    void setUseSortedSets(IloBool with) {
        impl().setUseSortedSets(with);
    }

    
    IloBool isUseSortedSets() const {
        return impl().isUseSortedSets();
    }

    
    void setDisplayWidth(IloInt value) {
        impl().setDisplayWidth(value);
    }

    
    IloInt getDisplayWidth() const {
        return impl().getDisplayWidth();
    }

    
    void setDisplayPrecision(IloInt value) {
        impl().setDisplayPrecision(value);
    }

    
    IloInt getDisplayPrecision() const {
        return impl().getDisplayPrecision();
    }

    
    void setDisplayWithIndex(IloBool with) {
        impl().setDisplayWithIndex(with);
    }

    
    IloBool isDisplayWithIndex() const {
        return impl().isDisplayWithIndex();
    }

    
    void setDisplayWithComponentName(IloBool with) {
        impl().setDisplayWithComponentName(with);
    }

    
    IloBool isDisplayWithComponentName() const {
        return impl().isDisplayWithComponentName();
    }

    
    void setDisplayOnePerLine(IloBool onoff) {
        impl().setDisplayOnePerLine(onoff);
    }

    
    IloBool isDisplayOnePerLine() const {
        return impl().isDisplayOnePerLine();
    }

	
    void setBigMapThreshold(IloInt value) {
        impl().setBigMapThreshold(value);
    }

    
    IloInt getBigMapThreshold() const {
        return impl().getBigMapThreshold();
    }

    
    void setMainEndEnabled(IloBool value) {
        impl().setMainEndEnabled(value);
    }

    
    IloBool isMainEndEnabled() const {
        return impl().isMainEndEnabled();
    }

    void setDelayExtraction(IloBool value) {
        impl().setDelayExtraction(value);
    }
    IloBool isDelayExtraction() const {
        return impl().isDelayExtraction();
    }

    
	void setSlicingCache(IloBool value) {
        impl().setSlicingCache(value);
    }
    
	IloBool hasSlicingCache() const {
        return impl().hasSlicingCache();
    }

    
    void setMemoryEmphasis(IloBool value) {
        impl().setMemoryEmphasis(value);
    }

    
    IloBool isMemoryEmphasis() const {
        return impl().isMemoryEmphasis();
    }

    
	const char* getResolverPath() const {
		return impl().getResolverPath();
	}

    
	void setResolverPath(const char* path) {
		impl().setResolverPath(path);
	}

    
	const char* getTmpDir() const {
		return impl().getTmpDir();
	}

    
	void setTmpDir(const char* path) {
		impl().setTmpDir(path);
	}

    
    void setKeepTmpFiles(IloBool value) {
        impl().setKeepTmpFiles(value);
    }

    
    IloBool isKeepTmpFiles() const {
        return impl().isKeepTmpFiles();
    }

    
    void setRelaxationLevel(IloInt value) {
        impl().setRelaxationLevel(value);
    }

    
    IloInt getRelaxationLevel() const {
        return impl().getRelaxationLevel();
    }

	
    IloBool hasProfiler() const {
        return impl().hasProfiler();
    }

    
    IloOplProfiler getProfiler() const {
        return IloOplProfiler(&(impl().getProfiler()));
    }

    
    void setProfiler(IloOplProfiler profiler) {
        impl().setProfiler(profiler.impl());
    }

    
    IloOplErrorHandler getErrorHandler() const {
        return IloOplErrorHandler(&(impl().getErrorHandler()));
    }

    
    void removeProfiler() {
        impl().removeProfiler();
    }

    
     void setResourceResolver(IloOplResourceResolver resolver) {
        impl().setResourceResolver(resolver.impl());
    }

    
    IloOplResourceResolver getResourceResolver() const {
        return &impl().getResourceResolver();
    }

    
    IloOplRunSettings getRunSettings() const {
        return IloOplRunSettings(getImpl());
    }

    
    void setUndefinedDataError(IloBool value) {
        impl().setUndefinedDataError(value);
    }

    
    IloBool isUndefinedDataError() const {
        return impl().isUndefinedDataError();
    }

    
    void setWithMultiEnv(IloBool value) {
    	impl().setWithMultiEnv(value);
    }

    
    IloBool isWithMultiEnv() const {
    	return impl().isWithMultiEnv();
    }

	
	void setWithJavaScript(IloBool value) {
		impl().setWithJavaScript(value);
	}

	
	IloBool isWithJavaScript() const {
		return impl().isWithJavaScript();
	}

	
	void setDo4dsxDebug(IloBool value) {
		impl().setDo4dsxDebug(value);
	}

	
	IloBool isDo4dsxDebug() const {
		return impl().isDo4dsxDebug();
	}
	    
    void setGC(IloBool value) {
    	impl().setGC(value);
    }

    
    IloBool usesGC() const {
    	return impl().usesGC();
    }
};

#ifdef _WIN32
#pragma pack(pop)
#endif

#endif

