@rem Compiles Corporate Command
@transcompiler /input:"Extensions\Corporate Hierarchy Vol 01\CorporateHierarchyVol01.xml" /output:TDBs\CorporateHierarchyVol01.tdb
@transcompiler /input:"Extensions\Corporate Expansion\CorporateExpansion.xml" /entities:"TDBs\CorporateHierarchyVol01.tdb" /output:TDBs\CorporateCommand.tdb
