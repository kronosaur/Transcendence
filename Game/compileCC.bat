@rem Compiles Corporate Command
@transcompiler /input:"c:\Users\George\Documents\Elysium\Extensions\CorporateHierarchyVol01\CorporateHierarchyVol01.xml" /output:TDBs\CorporateHierarchyVol01.tdb
@transcompiler /input:"c:\Users\George\Documents\Elysium\Extensions\CorporateCommand\CorporateExpansion.xml" /entities:"TDBs\CorporateHierarchyVol01.tdb" /output:TDBs\CorporateCommand.tdb
