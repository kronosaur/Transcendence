@rem Compiles Corporate Command
@transcompiler /input:"c:\Users\George\Documents\Elysium\CorporateHierarchyVol01\CorporateHierarchyVol01.xml" /output:TDBs\CorporateHierarchyVol01.tdb
@transcompiler /input:"c:\Users\George\Documents\Elysium\CorporateCommand\CorporateExpansion.xml" /entities:"TDBs\CorporateHierarchyVol01.tdb" /output:TDBs\CorporateCommand.tdb
