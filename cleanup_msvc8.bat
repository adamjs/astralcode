@echo This will clean up all intermediate/user-specific files for Astral. 
@echo ** You must close Visual C++ before running this file! **
@pause

@echo ========== Cleaning up... ==========
@del Astral.ncb
@del Astral.suo /AH
@del Astral\*.user
@rmdir Astral\objects\Release\ /S /Q
@rmdir Astral\objects\Debug\ /S /Q
@del Astral\lib\*.lib
@del Astral\lib\*.dll
@del Astral\lib\*.ilk
@del Astral\lib\*.idb
@del Astral\lib\*.pdb
@echo ============== Done! ===============
