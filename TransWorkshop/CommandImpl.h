//  CommandImpl.h
//
//  Implementation of commands
//  Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class CCmdHelp : public ITransCommand
    {
    public:
        CCmdHelp (ITransCommand *pCmd = NULL) :
                m_pCmd(pCmd)
            { }

        virtual ~CCmdHelp (void) override;

        virtual bool Execute (CSteamCtx &Ctx, CString *retsError) override;

    private:
        ITransCommand *m_pCmd;              //  Cmd to give help on (may be NULL)
    };

class CCmdUpload : public ITransCommand
    {
    public:
        virtual bool Execute (CSteamCtx &Ctx, CString *retsError) override;
        virtual bool Init (CXMLElement *pCmdLine, CString *retsError) override;
        virtual void PrintHelp (void) override;

    private:
		struct SUploadPaths
			{
			~SUploadPaths (void)
				{
				if (!sRoot.IsBlank())
					pathDeleteAll(sRoot);
				}

			CString sRoot;
			CString sContentPath;
			CString sPreviewPath;
			};

		bool CopyContentFile (const CString &sFilespec, const SUploadPaths &Paths, CString *retsError) const;
		bool CopyContentFiles (const CExtensionDirectory::SExtensionInfo &Info, const SUploadPaths &Paths, CString *retsError);
		bool CopyPreviewFile (const CString &sData, const CString &sDestFilespec, CString *retsError) const;
		bool CopyPreviewFiles (const CExtensionDirectory::SExtensionInfo &Info, const SUploadPaths &Paths, CString *retsPath, CString *retsError);
		bool CreateUploadPaths (SUploadPaths *retPaths, CString *retsError) const;
		bool ValidateExtension (const CString &sFilespec, DWORD *retdwUNID, CString *retsError) const;

        CString m_sFilespec;
		TArray<CString> m_Libraries;		//	List of dependent libraries to upload
		bool m_bUploadLibraries;			//	If TRUE, we auto-upload dependencies

        CExtensionDirectory m_Extensions;
    };