#pragma once

#include "Private.h"
#include "VadRoutines.h"
#include "BlackBoneDef.h"

typedef enum _ATTACHED_CONTEXT
{
    ContextNone,    // Running in system context
    ContextHost,    // Running in the context of host process
    ContextTarget,  // Running in the context of target process
} ATTACHED_CONTEXT;

/// <summary>
/// Process-specific data
/// </summary>
typedef struct _PROCESS_CONTEXT
{
    HANDLE pid;             // Process ID
    PVOID sharedPage;       // Address of kernel shared page
} PROCESS_CONTEXT, *PPROCESS_CONTEXT;


/// <summary>
/// Target - host correspondence
/// </summary>
typedef struct _PROCESS_MAP_ENTRY
{
    PROCESS_CONTEXT host;   // Hosting process context
    PROCESS_CONTEXT target; // Target process context

    PVOID pSharedPage;      // Address of kernel-shared page allocated from non-paged pool
    PMDL  pMDLShared;       // MDL of kernel-shared page

    HANDLE targetPipe;      // Hook pipe handle in target process

    LIST_ENTRY pageList;    // List of REMAP_ENTRY structures
} PROCESS_MAP_ENTRY, *PPROCESS_MAP_ENTRY;


/// <summary>
/// Mapped memory region info
/// </summary>
typedef struct _MAP_ENTRY
{
    LIST_ENTRY link;        // Linked list link
    ULONG_PTR originalPtr;  // Original memory address in target process
    ULONG_PTR newPtr;       // Mapped memory address in host process
    ULONG_PTR size;         // Region size

    PMDL    pMdl;           // Region MDL entry
    BOOLEAN locked;         // MDL is locked
    BOOLEAN shared;         // Regions has shared pages
    BOOLEAN readonly;       // Region must be mapped as readonly
} MAP_ENTRY, *PMAP_ENTRY;

extern DYNAMIC_DATA dynData;
extern RTL_AVL_TABLE g_ProcessPageTables;
extern KGUARDED_MUTEX g_globalLock;

/// <summary>
/// Map entire address space of target process into current
/// </summary>
/// <param name="pRemap">Mapping params</param>
/// <param name="ppEntry">Mapped context</param>
/// <returns>Status code</returns>
NTSTATUS BBMapMemory( IN PMAP_MEMORY pRemap, OUT PPROCESS_MAP_ENTRY* ppEntry );

/// <summary>
/// Map specific memory region
/// </summary>
/// <param name="pRegion">Region data</param>
/// <param name="pResult">Mapping results</param>
/// <returns>Status code</returns>
NTSTATUS BBMapMemoryRegion( IN PMAP_MEMORY_REGION pRegion, OUT PMAP_MEMORY_REGION_RESULT pResult );

/// <summary>
/// Unmap any mapped memory from host and target processes
/// </summary>
/// <param name="pUnmap">Request params</param>
/// <returns>Status code</returns>
NTSTATUS BBUnmapMemory( IN PUNMAP_MEMORY pUnmap );

/// <summary>
/// Unmap specific memory region
/// </summary>
/// <param name="pRegion">Region info</param>
/// <returns>Status ode</returns>
NTSTATUS BBUnmapMemoryRegion( IN PUNMAP_MEMORY_REGION pRegion );

/// <summary>
/// Calculate size required to store mapping info
/// </summary>
/// <param name="pList">Mapped regions list</param>
/// <param name="pSize">Resulting size</param>
/// <returns>Status code</returns>
NTSTATUS BBGetRequiredRemapOutputSize( IN PLIST_ENTRY pList, OUT PULONG_PTR pSize );

/// <summary>
/// Process termination handler
/// </summary>
/// <param name="ParentId">Parent PID</param>
/// <param name="ProcessId">PID</param>
/// <param name="Create">TRUE if process was created</param>
VOID BBProcessNotify( IN HANDLE ParentId, IN HANDLE ProcessId, IN BOOLEAN Create );

/// <summary>
/// Clear global process map table
/// </summary>
VOID BBCleanupProcessTable();

//
// AVL table routines
//
RTL_GENERIC_COMPARE_RESULTS AvlCompare( IN RTL_AVL_TABLE *Table, IN PVOID FirstStruct, IN PVOID SecondStruct );
PVOID AvlAllocate( IN RTL_AVL_TABLE *Table, IN CLONG ByteSize );
VOID AvlFree( IN RTL_AVL_TABLE *Table, IN PVOID Buffer );