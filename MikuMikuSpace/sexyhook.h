#ifdef _MSC_VER 
	//for VC++
	#pragma once
#endif
#ifndef ____SEXYHOOK____72
#define ____SEXYHOOK____72
//
//�ڍ����������ɍ쐬����
//SEXYHOOK
//
//
// by rti
//
//
// new BSD ���C�Z���X / NYSL ���C�Z���X �D���Ɏg���΂��������Ȃ��H ^q^
//

//include�Ƃ���`�Ƃ�.
#ifdef _WINDOWS_
	//windows
	#include <windows.h>
	#include <imagehlp.h>
	#ifdef _MSC_VER 
		#if _MSC_VER <= 1200 
			typedef int intptr_t; 
			typedef unsigned int uintptr_t; 
		#endif 
	#else 
	#endif 
#else
	//linux
	#include <stdarg.h>
	#include <inttypes.h>
	#include <sys/mman.h>
	#include <string.h>
	typedef void* PROC;
#endif

//�����I�Ƀ|�C���^�ɂ���(�ד�)
#ifdef __GNUC__
	// ... �����p�����Z
	static void* SEXYHOOK_DARKCAST(int dummy, ... )
	{
//		return *(void**)(&dummy + 1);
		va_list ap;
		va_start(ap, dummy);
		void* p = va_arg(ap, void*);
		va_end(ap);

		return p;
	}
#else
	//msvc �̂� gcc ���ƃG���[���ł邯�ǁA�A�[�L�e�N�`���ˑ����Ȃ��̂ōD����!
	template<typename _T> static void* SEXYHOOK_DARKCAST(int dummy, _T p)
	{
		return *reinterpret_cast<void**>(&p);
	}
#endif

//�u���[�N�|�C���g�ƃA�T�[�V����
#ifdef __GNUC__
	//gcc
	#define SEXYHOOK_BREAKPOINT	{ asm ("int $3") ; }
	#define SEXYHOOK_ASSERT(f) \
		if (!(f) ) \
		{ \
			asm ("int $3") ; \
		} 
#else
	//msvc
	#define SEXYHOOK_BREAKPOINT	{ DebugBreak(); }
	#define SEXYHOOK_ASSERT(f) \
		if (!(f) ) \
		{ \
			MSG msg;	\
			BOOL bQuit = PeekMessage(&msg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE);	\
			if (bQuit)	PostQuitMessage((int)msg.wParam);	\
			DebugBreak(); \
		} 
#endif

//�Ăяo���K��
#ifdef __GNUC__
	//gcc
	#define SEXYHOOK_STDCALL			__attribute__ ((stdcall))
	#define SEXYHOOK_CDECL				__attribute__ ((cdecl))
	#define SEXYHOOK_FASTCALL			__attribute__ ((regparm(3)))
	#define SEXYHOOK_FASTCALLN(N)		__attribute__ ((regparm(N)))
	#define SEXYHOOK_CLASS											//__thiscall __cdecl �Ƃ͈Ⴄ�炵��.
	#define SEXYHOOK_CLASS_STDCALL		__attribute__ ((stdcall))
	#define SEXYHOOK_CLASS_CDECL		__attribute__ ((cdecl))
	#define SEXYHOOK_CLASS_FASTCALL		__attribute__ ((regparm(3)))
	#define SEXYHOOK_CLASS_FASTCALLN(N)	__attribute__ ((regparm(N)))
#else
	//msvc
	#define SEXYHOOK_STDCALL		__stdcall
	#define SEXYHOOK_CDECL			__cdecl
	#define SEXYHOOK_FASTCALL		__fastcall
	#define SEXYHOOK_CLASS										//__thiscall __cdecl �Ƃ͈Ⴄ�炵��.
	#define SEXYHOOK_CLASS_STDCALL	__stdcall
	#define SEXYHOOK_CLASS_CDECL	__cdecl
	#define SEXYHOOK_CLASS_FASTCALL	__fastcall
#endif

//__LINE__�𕶎���ɂ��邽�߂̃}�N��
//�Q�l:http://oshiete1.goo.ne.jp/qa1219076.html
#define SEXYHOOK_LINE_TOSTR(x) SEXYHOOK_LINE_TOSTR_(x)
#define SEXYHOOK_LINE_TOSTR_(x) #x

#define SEXYHOOK_LINE_STRCAT(x,y) SEXYHOOK_LINE_STRCAT_(x,y)
#define SEXYHOOK_LINE_STRCAT_(x,y) x##y

//�}�N�����ēx�W�J����.
#define SEXYHOOK_REMACRO(MACRO)	MACRO

//�N���X���\�b�h�łȂ��ꍇ�́A static �w��ɂ��Ȃ��Ƃ����Ȃ��̂ŁA������ static����������o��.
#define SEXYHOOK_STDCALL__STATIC			static
#define SEXYHOOK_CDECL__STATIC				static
#define SEXYHOOK_FASTCALL__STATIC			static
#define SEXYHOOK_FASTCALLN__STATIC			static
#define SEXYHOOK_CLASS__STATIC	
#define SEXYHOOK_CLASS_STDCALL__STATIC	
#define SEXYHOOK_CLASS_CDECL__STATIC	
#define SEXYHOOK_CLASS_FASTCALL__STATIC	
#define SEXYHOOK_CLASS_FASTCALLN__STATIC	

//�N���X���\�b�h�̏ꍇ�� this ���B�����łȂ���� NULL ���쐬����.
#define SEXYHOOK_STDCALL__THIS			NULL
#define SEXYHOOK_CDECL__THIS			NULL
#define SEXYHOOK_FASTCALL__THIS			NULL
#define SEXYHOOK_FASTCALLN__THIS		NULL
#define SEXYHOOK_CLASS__THIS			this
#define SEXYHOOK_CLASS_STDCALL__THIS	this
#define SEXYHOOK_CLASS_CDECL__THIS		this
#define SEXYHOOK_CLASS_FASTCALL__THIS	this

//�֐��Ȃ̂����\�b�h�Ȃ̂��� typedef �̏���ւ���.
#define SEXYHOOK_STDCALL__HOOKDEF			
#define SEXYHOOK_CDECL__HOOKDEF			
#define SEXYHOOK_FASTCALL__HOOKDEF			
#define SEXYHOOK_FASTCALLN__HOOKDEF			
#define SEXYHOOK_CLASS__HOOKDEF				SEXYHOOKFuncBase::
#define SEXYHOOK_CLASS_STDCALL__HOOKDEF		SEXYHOOKFuncBase::
#define SEXYHOOK_CLASS_CDECL__HOOKDEF		SEXYHOOKFuncBase::
#define SEXYHOOK_CLASS_FASTCALL__HOOKDEF	SEXYHOOKFuncBase::
#define SEXYHOOK_CLASS_FASTCALLN__HOOKDEF	SEXYHOOKFuncBase::

//const����
#define SEXYHOOK_STDCALL__CONST			
#define SEXYHOOK_CDECL__CONST			
#define SEXYHOOK_FASTCALL__CONST			
#define SEXYHOOK_FASTCALLN__CONST			
#define SEXYHOOK_CLASS__CONST				const
#define SEXYHOOK_CLASS_STDCALL__CONST		const
#define SEXYHOOK_CLASS_CDECL__CONST			const
#define SEXYHOOK_CLASS_FASTCALL__CONST		const
#define SEXYHOOK_CLASS_FASTCALLN__CONST		const


//SEXYHOOK�{�� �N���X���N���X�Ƃ��Ď�������.
#define SEXYHOOK_BEGIN(RET,CALLTYPE,FUNCADDRESS,ARGS) \
	class SEXYHOOK_LINE_STRCAT(SEXYHOOKFunc,__LINE__) : public SEXYHOOKFuncBase { \
	public: \
		SEXYHOOK_LINE_STRCAT(SEXYHOOKFunc,__LINE__)() \
		{ \
		} \
		void Hook(void* inVCallThisPointer = NULL,void* inFuncAddress2 = NULL) \
		{ \
			*(getSexyhookThisPointer()) =  (uintptr_t)this; \
			void* p = inFuncAddress2; \
			if (!p) p = EvalLoad(#FUNCADDRESS); \
			if (!p) p = SEXYHOOK_DARKCAST(0,(HookDef)FUNCADDRESS); \
			FunctionHookFunction( \
					 p \
					,SEXYHOOK_DARKCAST(0,&SEXYHOOK_LINE_STRCAT(SEXYHOOKFunc,__LINE__)::HookFunction) \
					,SEXYHOOK_DARKCAST(0,&SEXYHOOK_LINE_STRCAT(SEXYHOOKFunc,__LINE__)::CallOrignalFunction) \
					,inVCallThisPointer \
					); \
		} \
		void UnHook() \
		{ \
			FunctionUnHookFunction(); \
		} \
		virtual ~SEXYHOOK_LINE_STRCAT(SEXYHOOKFunc,__LINE__)() \
		{ \
			FunctionUnHookFunction(); \
		} \
		typedef RET ( SEXYHOOK_REMACRO(CALLTYPE##__HOOKDEF)* HookDef) ARGS ; \
		typedef RET ( SEXYHOOK_REMACRO(CALLTYPE##__HOOKDEF)* HookDefConst) ARGS SEXYHOOK_REMACRO(CALLTYPE##__CONST); \
		static uintptr_t* getSexyhookThisPointer() \
		{ \
			static uintptr_t thisSaver = 0; \
			return &thisSaver ; \
		} \
		SEXYHOOK_REMACRO(CALLTYPE##__STATIC) RET  CALLTYPE CallOrignalFunction ARGS \
		{ \
			int mizumashi = 1 ; for(int i = 0 ; i < 1000 ; i++){ mizumashi= mizumashi * i + 1; }; \
			throw 0;\
		} \
		SEXYHOOK_REMACRO(CALLTYPE##__STATIC) RET  CALLTYPE HookFunction ARGS \
		{ \
			SEXYHOOK_LINE_STRCAT(SEXYHOOKFunc,__LINE__)* sexyhookThis = ((SEXYHOOK_LINE_STRCAT(SEXYHOOKFunc,__LINE__)*)(*getSexyhookThisPointer()) ); \
			void * sexyhookOrignalThis = (void*) SEXYHOOK_REMACRO(CALLTYPE##__THIS);

#define SEXYHOOK_END \
	} } SEXYHOOK_LINE_STRCAT(objectFUNCHook,__LINE__);\
	SEXYHOOK_LINE_STRCAT(objectFUNCHook,__LINE__).Hook

#define SEXYHOOK_END_AS() \
	} } 



//�ꎞ�I�Ƀt�b�N����߂�
#define SEXYHOOK_UNHOOK()	SEXYHOOKFuncBase::OriginalGarder sexyhook_temp_orignalgrad(sexyhookThis);  

//�I���W�i���� this pointer
#define SEXYHOOK_THIS(cast)	((cast) sexyhookOrignalThis )

//�����������肵�āA���O�������܂��Ȃ����ꍇ�̉���������[�U�[�ɒ񋟂���.
#define SEXYHOOK_AUTOCAST_CONST(func)	(HookDefConst)(func)

//DLL�����[�h����\���B
//����ANULL�Ԃ��Ă��邾���Ɍ������ˁB����B�����ǂˁA�����Ɠ���������S���Ăق����B
#define SEXYHOOK_DYNAMICLOAD(DLLNAME,FUNCNAME)	NULL

//�֐��t�b�N���s���ۂɏ���������̈�̃T�C�Y
typedef  char FUNCTIONHOOK_ASM[20] ;

//�֐�/���\�b�h���t�b�N����.
class SEXYHOOKFuncBase
{
	//�����������A�h���X�̐擪
	void*			 OrignalFunctionAddr;
	//�����������̈��ێ�����̈�
	FUNCTIONHOOK_ASM OrignalAsm;
	//�t�b�N����̂Ɏg�����R�[�h
	FUNCTIONHOOK_ASM HookAsm;
	//����������A�Z���u���̃T�C�Y
	int              HookAsmSize;
	//DLL�C���X�^���X
	void*			 APIHookDLLHandle;

public:
	//�I���W�i���̊֐��Ɉꎞ�I�ɖ߂�.
	class OriginalGarder
	{
		SEXYHOOKFuncBase* FuncBase;
	public:
		OriginalGarder(SEXYHOOKFuncBase* poolFuncBase)
		{
			this->FuncBase = poolFuncBase;
			this->FuncBase->BeginTempOrginal();
		}
		virtual ~OriginalGarder()
		{
			this->FuncBase->EndTempOrginal();
			this->FuncBase = NULL;
		}
	};

	//�ꎞ�I�Ƀt�b�N����߂�
	void BeginTempOrginal()
	{
		ReplaceFunction(this->OrignalFunctionAddr, this->OrignalAsm ,  NULL ,this->HookAsmSize);
	}
	//������x�t�b�N����
	void EndTempOrginal()
	{
		ReplaceFunction(this->OrignalFunctionAddr, this->HookAsm ,  NULL ,this->HookAsmSize);
	}


	//�֐��t�b�N���J�n����.
	void FunctionHookFunction(void * inUkeFunctionProc , void * inSemeFunctionProc ,void * inCallOrignalFunctionProc , void * inVCallThisPointer )
	{
		//�t�b�N����֐�(�U��)���J�n����A�h���X�����߂�
		uintptr_t semeFunctionAddr = CalcSemeFunctionAddress(inSemeFunctionProc );
		//�t�b�N�����֐�(��)���J�n����A�h���X�����߂�
		uintptr_t ukeFunctionAddr =  CalcUkeFunctionAddress(inUkeFunctionProc , inVCallThisPointer);


		//����������}�V����
		//�t�b�N�����֐��̐擪�����������āA�t�b�N���[�`���֐�����ڂ��悤�ɂ���B
		this->HookAsmSize = MakeTrampolineHookAsm( &this->HookAsm , ukeFunctionAddr , semeFunctionAddr) ;
		this->OrignalFunctionAddr = (void*)ukeFunctionAddr;
		ReplaceFunction(this->OrignalFunctionAddr , this->HookAsm , &this->OrignalAsm, this->HookAsmSize );

		//�I���W�i���̊֐����Ăяo���@�\.
		FUNCTIONHOOK_ASM orginalCallAsm;
		uintptr_t orignalCallFunctionAddr = CalcSemeFunctionAddress(inCallOrignalFunctionProc );
		int orginalCallAsmSize = MakeTrampolineHookAsm( &orginalCallAsm , orignalCallFunctionAddr , ukeFunctionAddr ) ;
		ReplaceFunction( (void*)orignalCallFunctionAddr , orginalCallAsm , NULL,orginalCallAsmSize);

		return ;
	}

	//�t�b�N����Ɏg��ꂽ�\�������āA
	//SEXYHOOK_APILOAD ��������Adll�̃��[�h���d���ށB eval�Ȃ�ʁAevil�B
	void* EvalLoad(const char * inAddress)
	{
		//����������.
		this->APIHookDLLHandle = NULL;

		const char * p = inAddress;

		p = strstr(p , "SEXYHOOK_DYNAMICLOAD");
		if (!p) return NULL;

		//��1���� DLL��
		const char * dll1 = strstr(p , "\"");
		if (!dll1) return NULL;
		const char * dll2 = strstr(dll1+1 , "\"");
		if (!dll2) return NULL;

		//��2���� �֐���
		const char * func1 = strstr(dll2+1 , "\"");
		if (!func1) return NULL;
		const char * func2 = strstr(func1+1 , "\"");
		if (!func2) return NULL;


		//��������o�b�t�@�ɕ�������.
		char buffer[1024];
		int len = (int)(func2 - inAddress) + 1;
		if (len >= sizeof(buffer) )
		{
			//������f�J����.
			return NULL;
		}
		memcpy(buffer , inAddress , len );

		//�I�[���ߍ���ŁA
		buffer[(int)(dll2 - inAddress) ] = '\0';
		buffer[(int)(func2 - inAddress) ] = '\0';
		//�擪�̕ێ�
		const char * apiHookDLLName = buffer + (int)(dll1 - inAddress + 1);
		const char * apiHookFuncName = buffer + (int)(func1 - inAddress + 1);
#ifdef _WINDOWS_
		//DLL�ǂݍ���
		HMODULE mod = LoadLibraryA(apiHookDLLName);
		if ( mod == NULL )
		{
			//DLL���ǂݍ��߂܂���!
			SEXYHOOK_BREAKPOINT;
			return NULL;
		}
		//API�̃A�h���X�����߂�.
		void* orignalProc = (void*)GetProcAddress(mod , apiHookFuncName );
		if ( orignalProc == NULL )
		{
			//API�����[�h�ł��܂���B
			SEXYHOOK_BREAKPOINT;
			FreeLibrary(mod);
			return NULL;
		}
		//�J���̂��߂̋L�^.
		this->APIHookDLLHandle = (void*)mod;
		
		return orignalProc;
#else
		return NULL;
#endif
	}

	//�֐��t�b�N����߂�
	void FunctionUnHookFunction()
	{
		if (this->OrignalFunctionAddr == NULL)
		{
			return ;
		}
		ReplaceFunction(this->OrignalFunctionAddr , this->OrignalAsm , NULL,this->HookAsmSize);
		this->OrignalFunctionAddr = NULL;

		//API���A�����[�h���Ȃ��Ă͂����Ȃ��ꍇ�͊J������.
		if(this->APIHookDLLHandle)
		{
#ifdef _WINDOWS_
			FreeLibrary((HMODULE)this->APIHookDLLHandle);
#endif
			this->APIHookDLLHandle = NULL;
		}
	}
	//�t�b�N�����֐��̃A�h���X��Ԃ�.
	void* getOrignalFunctionAddr()  const
	{
		return this->OrignalFunctionAddr;
	}
private:

	//�t�b�N����֐�(�U��)���J�n����A�h���X�����߂�
	uintptr_t CalcSemeFunctionAddress(void * inSemeFunctionProc )
	{
		//�t�b�N�֐����J�n������΃A�h���X
		if (*((unsigned char*)inSemeFunctionProc+0) == 0xe9)
		{
			//�t�b�N�֐��� ILT�o�R�Ŕ��ł���ꍇ
			//0xe9 call [4�o�C�g���΃A�h���X]          4�o�C�g���΂�64bit �ł��ς�炸
			uintptr_t jmpaddress = *((unsigned long*)((unsigned char*)inSemeFunctionProc+1));
			return (((uintptr_t)inSemeFunctionProc) + jmpaddress) + 5;
		}
		else
		{
			//���A�v���O�����̈�ɔ��ł���ꍇ
			return (uintptr_t)inSemeFunctionProc ;
		}
	}

	//MSVC++ �� vcall����͂���.
	uintptr_t CalcVCall(uintptr_t overraideFunctionAddr , void * inVCallThisPointer )
	{

		//���z�֐��� vcall�������ꍇ...
		int vcallhead = 0;
		if (
				*((unsigned char*)overraideFunctionAddr+0) == 0x8B
			&&	*((unsigned char*)overraideFunctionAddr+1) == 0x01
			&&	*((unsigned char*)overraideFunctionAddr+2) == 0xFF
			)
		{
			vcallhead = 3;
		}
		else if (
				*((unsigned char*)overraideFunctionAddr+0) == 0x48
			&&	*((unsigned char*)overraideFunctionAddr+1) == 0x8B
			&&	*((unsigned char*)overraideFunctionAddr+2) == 0x01
			&&	*((unsigned char*)overraideFunctionAddr+3) == 0xFF
			)
		{
			vcallhead = 4;
		}
		else
		{
			//vcall�ł͂Ȃ�.
			return 0;
		}

		int plusAddress = 0;
		if (*((unsigned char*)overraideFunctionAddr+vcallhead ) == 0x20)
		{
			//[[this] + 0] �ɃW�����v
			plusAddress = 0;
		}
		else if (*((unsigned char*)overraideFunctionAddr+vcallhead ) == 0x60)
		{
			//[[this] + ?] �ɃW�����v
			plusAddress = (int) *((unsigned char*)overraideFunctionAddr+ (vcallhead + 1) );	//4�o�C�g�ڂ�1�o�C�g�������Z����l
		}
		else
		{
			//[[this] + ?] �ɃW�����v���v�Z�o���܂���ł���...
			SEXYHOOK_BREAKPOINT;
		}
		//C����̂����������ŁA�|�C���^�͌^���v���X���Ă��܂��̂ŁA�|�C���^�̃T�C�Y�Ŋ����Ƃ�.
		plusAddress = plusAddress / sizeof(void*);

		//���̂悤�Ȋ֐��Ɉꎞ�I�ɔ�΂���Ă���ꍇ...
		//			vcall:
		//			00402BA0   mov         eax,dword ptr [ecx]
		//			00402BA2   jmp         dword ptr [eax]
		//8B 01 FF 20
		//
		// or
		//
		//004025D0   mov         eax,dword ptr [ecx]
		//004025D2   jmp         dword ptr [eax+4]
		//8B 01 FF 60 04
		if ( inVCallThisPointer == NULL )
		{
			//vcall�̃t�b�N�ɂ́A this�|�C���^���K�v�ł��B
			//SEXYHOOK_CLASS_END_VCALL(thisClass) �𗘗p���Ă��������B
			SEXYHOOK_BREAKPOINT;
		}

		/*
			//�����������Z��������  inVCallThisPointer = &this;
		_asm
		{
			mov ecx,inVCallThisPointer;
			mov ecx,[ecx];
			mov ecx,[ecx];
			mov overraideFunctionAddr,ecx;
		}

					or 
		_asm
		{
			mov ecx,inVCallThisPointer;
			mov ecx,[ecx];
			mov ecx,[ecx+4];			//+? �͒�`���ꂽ�֐��� virtual�̐������������
			mov overraideFunctionAddr,ecx;
		}
		*/
		//��������Ȋ���,,,�����Ă���L���X�g��.
		overraideFunctionAddr = (uintptr_t) *((void**)*((void***)inVCallThisPointer) + plusAddress);
		//�����ɂ���̂�  �֐��̖{�� jmp �ւ̖��߂̂͂�.
		if (*((unsigned char*)overraideFunctionAddr+0) == 0xe9)
		{
			//���łȂ̂Ŋ֐��̒������������邽�߁A�֐��̎��̂ւ̃A�h���X�����߂�.
			uintptr_t jmpaddress = *((unsigned long*)((unsigned char*)overraideFunctionAddr+1));
			overraideFunctionAddr = (((uintptr_t)overraideFunctionAddr) + jmpaddress) + 5;	//+5�� e9 00 00 00 00 (ILT�̃T�C�Y)
		}

		return overraideFunctionAddr;
	}


	//�t�b�N�����֐�(��)���J�n����A�h���X�����߂�
	uintptr_t CalcUkeFunctionAddress(void * inUkeFunctionProc , void * inVCallThisPointer)
	{
		#ifdef __GNUC__
			//gcc�ł͉��z�֐��̃|�C���^���擾���悤�Ƃ���ƁA vtable ����� index ��Ԃ��Ă��܂��B
			if ( (uintptr_t)inUkeFunctionProc < 100 )
			{
				//�N���X�̃C���X�^���X(this�|�C���^)���n����Ă���΁Aindex������̂̏ꏊ���v�Z�\�B
				if (inVCallThisPointer == NULL)
				{
					//this���Ȃ��Ȃ�v�Z�s�\�Ȃ̂ŁA�Ƃ肠�����Ƃ߂�.
					SEXYHOOK_BREAKPOINT;
				}
				//this������Όv�Z���ăA�h���X�����߂�.
				//�Q�l: http://d.hatena.ne.jp/Seasons/20090208/1234115944
				uintptr_t* vtable = (uintptr_t*) (*((uintptr_t*)inVCallThisPointer));
				//�Ƃ肠�����A (index - 1) / sizeof(void*) �ŃA�h���X�����܂�݂���.
				//�R���ł����Ă���̂��s�������ǁA�Ƃ肠����������.
				uintptr_t index = ((uintptr_t)inUkeFunctionProc - 1) / sizeof(void*);

				//vtable ���� index ���v�Z����.
				inUkeFunctionProc = (void*) (vtable[index] );
			}
		#endif
		
		//�t�b�N�����֐��̎��̈�����߂�.
		uintptr_t overraideFunctionAddr = 0;
		if (*((unsigned char*)inUkeFunctionProc+0) == 0xe9)
		{
			//�t�b�N�֐��� ILT�o�R�Ŕ��ł���ꍇ
			//0xe9 call [4�o�C�g���΃A�h���X]
			uintptr_t jmpaddress = *((unsigned long*)((unsigned char*)inUkeFunctionProc+1));
			overraideFunctionAddr = (((uintptr_t)inUkeFunctionProc) + jmpaddress) + 5;	//+5�� e9 00 00 00 00 (ILT�̃T�C�Y)
		}
		else
		{
			//���A�v���O�����̈�ɔ��ł���ꍇ
			overraideFunctionAddr = (uintptr_t)inUkeFunctionProc;
		}

		//���z�֐��� vcall�������ꍇ...
		uintptr_t vcallFunctionAddr = this->CalcVCall(overraideFunctionAddr,inVCallThisPointer);
		if (vcallFunctionAddr != 0)
		{
			return vcallFunctionAddr;
		}
		return overraideFunctionAddr;
	}


	//�g�����|�����t�b�N�̍쐬
	int MakeTrampolineHookAsm(FUNCTIONHOOK_ASM* outBuffer , uintptr_t inUkeFunctionAddr, uintptr_t inSemeFunctionAddr) const
	{
#if (_WIN64 || __x86_64__)
		//�Q�l http://www.artonx.org/diary/200809.html
		//     http://hrb.osask.jp/wiki/?faq/asm
		if (inSemeFunctionAddr - inUkeFunctionAddr < 0x80000000)
		{//2G�ȉ���jmp
			// 0xe9 ���΃A�h���X   = 5�o�C�g �̖���
			*((unsigned char*)outBuffer+0) = 0xe9;	//�ߗ׃W�����v JMP
			*((unsigned long*)((unsigned char*)outBuffer+1)) = (unsigned long) (inSemeFunctionAddr - inUkeFunctionAddr - 5);	//-5��jmp���ߎ��M�̃T�C�Y

			return 5;
		}
		else
		{//2G�ȏ��jmp
			//Nikolay Igotti����̕����𗘗p����. 14�o�C�g�̖���
			//push 64bit ; ret; �Ŕ�����B ret �Ȃ̂� ���΂ł͂Ȃ���΃A�h���X(inSemeFunctionAddr)������B
			//push 64bit �����̂܂܂��Ƃł��Ȃ��̂ŁA�������� push ����B
			//http://blogs.sun.com/nike/entry/long_absolute_jumps_on_amd64
			//http://www.ragestorm.net/blogs/?p=107
			*((unsigned char*)outBuffer+0) = 0x68;	//push imm32, subs 8 from rsp
			*((unsigned long*)((unsigned char*)outBuffer+1)) = 0x00000000ffffffff & inSemeFunctionAddr;

			*((unsigned char*)outBuffer+5) = 0xc7;	//mov imm32, 4(%rsp)
			*((unsigned char*)outBuffer+6) = 0x44;
			*((unsigned char*)outBuffer+7) = 0x24;
			*((unsigned char*)outBuffer+8) = 0x04;
			*((unsigned long*)((unsigned char*)outBuffer+9)) = inSemeFunctionAddr >> 32;

			*((unsigned char*)outBuffer+13) = 0xc3;	//ret

			return 14;
		}
#else
//#elif (_WIN32 || __i386__)
		//i386

		// 0xe9 ���΃A�h���X   = 5�o�C�g �̖���
		*((unsigned char*)outBuffer+0) = 0xe9;	//�ߗ׃W�����v JMP
		*((unsigned long*)((unsigned char*)outBuffer+1)) = (unsigned long) (inSemeFunctionAddr - inUkeFunctionAddr - 5);	//-5��jmp���ߎ��M�̃T�C�Y

		return 5;
#endif
	}



	bool ReplaceFunction(void* inAddr , void* inNewSrc ,  void* outOldSrc , int size)
	{
		//���荞�ރR�[�h���������݂܂��B

		if (outOldSrc != NULL)
		{
			//�o�b�N�A�b�v
			memcpy(outOldSrc , inAddr , size );
		}

		#ifdef _WINDOWS_
			//������������
			unsigned long oldProtect = 0;
			VirtualProtect( inAddr , size , PAGE_EXECUTE_READWRITE , &oldProtect);
			//��������
			memcpy(inAddr , inNewSrc , size );
			//���������ɖ߂�
			VirtualProtect( inAddr , size , oldProtect , &oldProtect);
		#else
			//mprotect �ɓn���l�́A�y�[�W���E(4K)�ɍ��킹��
			//http://d.hatena.ne.jp/kanbayashi/20081005/p2
			void *pageAddr = (void*)(((uintptr_t)inAddr) & 0xFFFFF000);
			//������������
			mprotect( pageAddr, 0x1000 , PROT_READ | PROT_WRITE | PROT_EXEC);
			//��������
			memcpy(inAddr , inNewSrc , size );
			//���Ƃɖ߂������񂾂��ǂǂ�����΂����́H
		#endif
		return true;
	}
};

#endif //____SEXYHOOK____72
