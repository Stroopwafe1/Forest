#include <sstream>
#include <fstream>
#include <map>
#include <algorithm>
#include <iostream>
#include "X86_64LinuxYasmCompiler.hpp"

X86_64LinuxYasmCompiler::X86_64LinuxYasmCompiler() {
	syscallTable = {
	{"SYS_READ", 0},
	{"SYS_WRITE", 1},
	{"SYS_OPEN", 2},
	{"SYS_CLOSE", 3},
	{"SYS_STAT", 4},
	{"SYS_FSTAT", 5},
	{"SYS_LSTAT", 6},
	{"SYS_POLL", 7},
	{"SYS_LSEEK", 8},
	{"SYS_MMAP", 9},
	{"SYS_MPROTECT", 10},
	{"SYS_MUNMAP", 11},
	{"SYS_BRK", 12},
	{"SYS_RT_SIGACTION", 13},
	{"SYS_RT_SIGPROCMASK", 14},
	{"SYS_RT_SIGRETURN", 15},
	{"SYS_IOCTL", 16},
	{"SYS_PREAD64", 17},
	{"SYS_PWRITE64", 18},
	{"SYS_READV", 19},
	{"SYS_WRITEV", 20},
	{"SYS_ACCESS", 21},
	{"SYS_PIPE", 22},
	{"SYS_SELECT", 23},
	{"SYS_SCHED_YIELD", 24},
	{"SYS_MREMAP", 25},
	{"SYS_MSYNC", 26},
	{"SYS_MINCORE", 27},
	{"SYS_MADVISE", 28},
	{"SYS_SHMGET", 29},
	{"SYS_SHMAT", 30},
	{"SYS_SHMCTL", 31},
	{"SYS_DUP", 32},
	{"SYS_DUP2", 33},
	{"SYS_PAUSE", 34},
	{"SYS_NANOSLEEP", 35},
	{"SYS_GETITIMER", 36},
	{"SYS_ALARM", 37},
	{"SYS_SETITIMER", 38},
	{"SYS_GETPID", 39},
	{"SYS_SENDFILE", 40},
	{"SYS_SOCKET", 41},
	{"SYS_CONNECT", 42},
	{"SYS_ACCEPT", 43},
	{"SYS_SENDTO", 44},
	{"SYS_RECVFROM", 45},
	{"SYS_SENDMSG", 46},
	{"SYS_RECVMSG", 47},
	{"SYS_SHUTDOWN", 48},
	{"SYS_BIND", 49},
	{"SYS_LISTEN", 50},
	{"SYS_GETSOCKNAME", 51},
	{"SYS_GETPEERNAME", 52},
	{"SYS_SOCKETPAIR", 53},
	{"SYS_SETSOCKOPT", 54},
	{"SYS_GETSOCKOPT", 55},
	{"SYS_CLONE", 56},
	{"SYS_FORK", 57},
	{"SYS_VFORK", 58},
	{"SYS_EXECVE", 59},
	{"SYS_EXIT", 60},
	{"SYS_WAIT4", 61},
	{"SYS_KILL", 62},
	{"SYS_UNAME", 63},
	{"SYS_SEMGET", 64},
	{"SYS_SEMOP", 65},
	{"SYS_SEMCTL", 66},
	{"SYS_SHMDT", 67},
	{"SYS_MSGGET", 68},
	{"SYS_MSGSND", 69},
	{"SYS_MSGRCV", 70},
	{"SYS_MSGCTL", 71},
	{"SYS_FCNTL", 72},
	{"SYS_FLOCK", 73},
	{"SYS_FSYNC", 74},
	{"SYS_FDATASYNC", 75},
	{"SYS_TRUNCATE", 76},
	{"SYS_FTRUNCATE", 77},
	{"SYS_GETDENTS", 78},
	{"SYS_GETCWD", 79},
	{"SYS_CHDIR", 80},
	{"SYS_FCHDIR", 81},
	{"SYS_RENAME", 82},
	{"SYS_MKDIR", 83},
	{"SYS_RMDIR", 84},
	{"SYS_CREAT", 85},
	{"SYS_LINK", 86},
	{"SYS_UNLINK", 87},
	{"SYS_SYMLINK", 88},
	{"SYS_READLINK", 89},
	{"SYS_CHMOD", 90},
	{"SYS_FCHMOD", 91},
	{"SYS_CHOWN", 92},
	{"SYS_FCHOWN", 93},
	{"SYS_LCHOWN", 94},
	{"SYS_UMASK", 95},
	{"SYS_GETTIMEOFDAY", 96},
	{"SYS_GETRLIMIT", 97},
	{"SYS_GETRUSAGE", 98},
	{"SYS_SYSINFO", 99},
	{"SYS_TIMES", 100},
	{"SYS_PTRACE", 101},
	{"SYS_GETUID", 102},
	{"SYS_SYSLOG", 103},
	{"SYS_GETGID", 104},
	{"SYS_SETUID", 105},
	{"SYS_SETGID", 106},
	{"SYS_GETEUID", 107},
	{"SYS_GETEGID", 108},
	{"SYS_SETPGID", 109},
	{"SYS_GETPPID", 110},
	{"SYS_GETPGRP", 111},
	{"SYS_SETSID", 112},
	{"SYS_SETREUID", 113},
	{"SYS_SETREGID", 114},
	{"SYS_GETGROUPS", 115},
	{"SYS_SETGROUPS", 116},
	{"SYS_SETRESUID", 117},
	{"SYS_GETRESUID", 118},
	{"SYS_SETRESGID", 119},
	{"SYS_GETRESGID", 120},
	{"SYS_GETPGID", 121},
	{"SYS_SETFSUID", 122},
	{"SYS_SETFSGID", 123},
	{"SYS_GETSID", 124},
	{"SYS_CAPGET", 125},
	{"SYS_CAPSET", 126},
	{"SYS_RT_SIGPENDING", 127},
	{"SYS_RT_SIGTIMEDWAIT", 128},
	{"SYS_RT_SIGQUEUEINFO", 129},
	{"SYS_RT_SIGSUSPEND", 130},
	{"SYS_SIGALTSTACK", 131},
	{"SYS_UTIME", 132},
	{"SYS_MKNOD", 133},
	{"SYS_USELIB", 134},
	{"SYS_PERSONALITY", 135},
	{"SYS_USTAT", 136},
	{"SYS_STATFS", 137},
	{"SYS_FSTATFS", 138},
	{"SYS_SYSFS", 139},
	{"SYS_GETPRIORITY", 140},
	{"SYS_SETPRIORITY", 141},
	{"SYS_SCHED_SETPARAM", 142},
	{"SYS_SCHED_GETPARAM", 143},
	{"SYS_SCHED_SETSCHEDULER", 144},
	{"SYS_SCHED_GETSCHEDULER", 145},
	{"SYS_SCHED_GET_PRIORITY_MAX", 146},
	{"SYS_SCHED_GET_PRIORITY_MIN", 147},
	{"SYS_SCHED_RR_GET_INTERVAL", 148},
	{"SYS_MLOCK", 149},
	{"SYS_MUNLOCK", 150},
	{"SYS_MLOCKALL", 151},
	{"SYS_MUNLOCKALL", 152},
	{"SYS_VHANGUP", 153},
	{"SYS_MODIFY_LDT", 154},
	{"SYS_PIVOT_ROOT", 155},
	{"SYS__SYSCTL", 156},
	{"SYS_PRCTL", 157},
	{"SYS_ARCH_PRCTL", 158},
	{"SYS_ADJTIMEX", 159},
	{"SYS_SETRLIMIT", 160},
	{"SYS_CHROOT", 161},
	{"SYS_SYNC", 162},
	{"SYS_ACCT", 163},
	{"SYS_SETTIMEOFDAY", 164},
	{"SYS_MOUNT", 165},
	{"SYS_UMOUNT2", 166},
	{"SYS_SWAPON", 167},
	{"SYS_SWAPOFF", 168},
	{"SYS_REBOOT", 169},
	{"SYS_SETHOSTNAME", 170},
	{"SYS_SETDOMAINNAME", 171},
	{"SYS_IOPL", 172},
	{"SYS_IOPERM", 173},
	{"SYS_CREATE_MODULE", 174},
	{"SYS_INIT_MODULE", 175},
	{"SYS_DELETE_MODULE", 176},
	{"SYS_GET_KERNEL_SYMS", 177},
	{"SYS_QUERY_MODULE", 178},
	{"SYS_QUOTACTL", 179},
	{"SYS_NFSSERVCTL", 180},
	{"SYS_GETPMSG", 181},
	{"SYS_PUTPMSG", 182},
	{"SYS_AFS_SYSCALL", 183},
	{"SYS_TUXCALL", 184},
	{"SYS_SECURITY", 185},
	{"SYS_GETTID", 186},
	{"SYS_READAHEAD", 187},
	{"SYS_SETXATTR", 188},
	{"SYS_LSETXATTR", 189},
	{"SYS_FSETXATTR", 190},
	{"SYS_GETXATTR", 191},
	{"SYS_LGETXATTR", 192},
	{"SYS_FGETXATTR", 193},
	{"SYS_LISTXATTR", 194},
	{"SYS_LLISTXATTR", 195},
	{"SYS_FLISTXATTR", 196},
	{"SYS_REMOVEXATTR", 197},
	{"SYS_LREMOVEXATTR", 198},
	{"SYS_FREMOVEXATTR", 199},
	{"SYS_TKILL", 200},
	{"SYS_TIME", 201},
	{"SYS_FUTEX", 202},
	{"SYS_SCHED_SETAFFINITY", 203},
	{"SYS_SCHED_GETAFFINITY", 204},
	{"SYS_SET_THREAD_AREA", 205},
	{"SYS_IO_SETUP", 206},
	{"SYS_IO_DESTROY", 207},
	{"SYS_IO_GETEVENTS", 208},
	{"SYS_IO_SUBMIT", 209},
	{"SYS_IO_CANCEL", 210},
	{"SYS_GET_THREAD_AREA", 211},
	{"SYS_LOOKUP_DCOOKIE", 212},
	{"SYS_EPOLL_CREATE", 213},
	{"SYS_EPOLL_CTL_OLD", 214},
	{"SYS_EPOLL_WAIT_OLD", 215},
	{"SYS_REMAP_FILE_PAGES", 216},
	{"SYS_GETDENTS64", 217},
	{"SYS_SET_TID_ADDRESS", 218},
	{"SYS_RESTART_SYSCALL", 219},
	{"SYS_SEMTIMEDOP", 220},
	{"SYS_FADVISE64", 221},
	{"SYS_TIMER_CREATE", 222},
	{"SYS_TIMER_SETTIME", 223},
	{"SYS_TIMER_GETTIME", 224},
	{"SYS_TIMER_GETOVERRUN", 225},
	{"SYS_TIMER_DELETE", 226},
	{"SYS_CLOCK_SETTIME", 227},
	{"SYS_CLOCK_GETTIME", 228},
	{"SYS_CLOCK_GETRES", 229},
	{"SYS_CLOCK_NANOSLEEP", 230},
	{"SYS_EXIT_GROUP", 231},
	{"SYS_EPOLL_WAIT", 232},
	{"SYS_EPOLL_CTL", 233},
	{"SYS_TGKILL", 234},
	{"SYS_UTIMES", 235},
	{"SYS_VSERVER", 236},
	{"SYS_MBIND", 237},
	{"SYS_SET_MEMPOLICY", 238},
	{"SYS_GET_MEMPOLICY", 239},
	{"SYS_MQ_OPEN", 240},
	{"SYS_MQ_UNLINK", 241},
	{"SYS_MQ_TIMEDSEND", 242},
	{"SYS_MQ_TIMEDRECEIVE", 243},
	{"SYS_MQ_NOTIFY", 244},
	{"SYS_MQ_GETSETATTR", 245},
	{"SYS_KEXEC_LOAD", 246},
	{"SYS_WAITID", 247},
	{"SYS_ADD_KEY", 248},
	{"SYS_REQUEST_KEY", 249},
	{"SYS_KEYCTL", 250},
	{"SYS_IOPRIO_SET", 251},
	{"SYS_IOPRIO_GET", 252},
	{"SYS_INOTIFY_INIT", 253},
	{"SYS_INOTIFY_ADD_WATCH", 254},
	{"SYS_INOTIFY_RM_WATCH", 255},
	{"SYS_MIGRATE_PAGES", 256},
	{"SYS_OPENAT", 257},
	{"SYS_MKDIRAT", 258},
	{"SYS_MKNODAT", 259},
	{"SYS_FCHOWNAT", 260},
	{"SYS_FUTIMESAT", 261},
	{"SYS_NEWFSTATAT", 262},
	{"SYS_UNLINKAT", 263},
	{"SYS_RENAMEAT", 264},
	{"SYS_LINKAT", 265},
	{"SYS_SYMLINKAT", 266},
	{"SYS_READLINKAT", 267},
	{"SYS_FCHMODAT", 268},
	{"SYS_FACCESSAT", 269},
	{"SYS_PSELECT6", 270},
	{"SYS_PPOLL", 271},
	{"SYS_UNSHARE", 272},
	{"SYS_SET_ROBUST_LIST", 273},
	{"SYS_GET_ROBUST_LIST", 274},
	{"SYS_SPLICE", 275},
	{"SYS_TEE", 276},
	{"SYS_SYNC_FILE_RANGE", 277},
	{"SYS_VMSPLICE", 278},
	{"SYS_MOVE_PAGES", 279},
	{"SYS_UTIMENSAT", 280},
	{"SYS_EPOLL_PWAIT", 281},
	{"SYS_SIGNALFD", 282},
	{"SYS_TIMERFD_CREATE", 283},
	{"SYS_EVENTFD", 284},
	{"SYS_FALLOCATE", 285},
	{"SYS_TIMERFD_SETTIME", 286},
	{"SYS_TIMERFD_GETTIME", 287},
	{"SYS_ACCEPT4", 288},
	{"SYS_SIGNALFD4", 289},
	{"SYS_EVENTFD2", 290},
	{"SYS_EPOLL_CREATE1", 291},
	{"SYS_DUP3", 292},
	{"SYS_PIPE2", 293},
	{"SYS_INOTIFY_INIT1", 294},
	{"SYS_PREADV", 295},
	{"SYS_PWRITEV", 296},
	{"SYS_RT_TGSIGQUEUEINFO", 297},
	{"SYS_PERF_EVENT_OPEN", 298},
	{"SYS_RECVMMSG", 299},
	{"SYS_FANOTIFY_INIT", 300},
	{"SYS_FANOTIFY_MARK", 301},
	{"SYS_PRLIMIT64", 302},
	{"SYS_NAME_TO_HANDLE_AT", 303},
	{"SYS_OPEN_BY_HANDLE_AT", 304},
	{"SYS_CLOCK_ADJTIME", 305},
	{"SYS_SYNCFS", 306},
	{"SYS_SENDMMSG", 307},
	{"SYS_SETNS", 308},
	{"SYS_GETCPU", 309},
	{"SYS_PROCESS_VM_READV", 310},
	{"SYS_PROCESS_VM_WRITEV", 311},
	{"SYS_KCMP", 312},
	{"SYS_FINIT_MODULE", 313},
	{"SYS_SCHED_SETATTR", 314},
	{"SYS_SCHED_GETATTR", 315},
	{"SYS_RENAMEAT2", 316},
	{"SYS_SECCOMP", 317},
	{"SYS_GETRANDOM", 318},
	{"SYS_MEMFD_CREATE", 319},
	{"SYS_KEXEC_FILE_LOAD", 320},
	{"SYS_BPF", 321},
	{"SYS_EXECVEAT", 322},
	{"SYS_USERFAULTFD", 323},
	{"SYS_MEMBARRIER", 324},
	{"SYS_MLOCK2", 325},
	{"SYS_COPY_FILE_RANGE", 326},
	{"SYS_PREADV2", 327},
	{"SYS_PWRITEV2", 328},
	{"SYS_PKEY_MPROTECT", 329},
	{"SYS_PKEY_ALLOC", 330},
	{"SYS_PKEY_FREE", 331},
	{"SYS_STATX", 332},
	{"SYS_NOT IMPLEMENTED", 333},
	{"SYS_RSEQ", 334},
	{"SYS_IO_URING_SETUP", 425},
	{"SYS_IO_URING_ENTER", 426},
	{"SYS_PIDFD_OPEN", 434},
	{"SYS_CLONE3", 435},
	{"SYS_CLOSE_RANGE", 436},
	{"SYS_FACCESSAT2", 439},
	};
}


int X86_64LinuxYasmCompiler::addToSymbols(int* offset, const Variable& variable, const std::string& reg, bool isGlobal) {
	int result = getSizeFromType(variable.mType);

	if (!isGlobal) {
		std::string checkReg = reg.substr(0, 3);
		char op = reg.size() == 4 ? reg[3] : '-';
		if (checkReg == "rbp") {
			if (op == '-')
				(*offset) -= 1 << result;
			else
				(*offset) += 1 << result;
		}
		symbolTable.insert(std::make_pair(variable.mName, SymbolInfo {checkReg, *offset, variable.mType, result, isGlobal}));
	} else {
		symbolTable.insert(std::make_pair(variable.mName, SymbolInfo {variable.mName, 0, variable.mType, result, isGlobal}));
	}
	return result;
}

void X86_64LinuxYasmCompiler::compile(fs::path& filePath, const Programme& p, const CompileContext& ctx) {
	fs::path fileName = filePath.stem();
	std::string parentPath = filePath.parent_path().string();
	fs::path buildPath = filePath.parent_path() / "build";
	fs::create_directory(buildPath);
	fs::path outPath = buildPath;
	outPath /= fileName.concat(".asm");
	std::ofstream outfile;
	outfile.open(outPath);

	std::vector<Variable> constantVars;
	std::vector<Variable> initVars;
	std::vector<Variable> otherVars;

	for (const auto& kv : p.variables) {
		auto& c = ctx.getSymbolConvention(kv.first);
		if (c != ctx.conventionsEnd) {
			if (((*c).modifiers & Modifiers::CONSTANT) == Modifiers::CONSTANT) {
				constantVars.push_back(kv.second);
			} else if (!kv.second.mValues.empty()) {
				initVars.push_back(kv.second);
			} else {
				otherVars.push_back(kv.second);
			}
		}
	}

	labelCount = 0;
	ifCount = 0;
	if (!constantVars.empty()) {
		outfile << "section .rodata" << std::endl;
		for (const auto& constVar : constantVars) {
			addToSymbols(nullptr, constVar, constVar.mName, true);
			outfile << "\t" << constVar.mName << " " << getDefineBytes(constVar.mType.byteSize) << " ";
			for (int i = 0; i < constVar.mValues.size(); i++) {
				outfile << constVar.mValues[i]->mValue.mText;
				if (i != constVar.mValues.size() - 1)
					outfile << ", ";
			}
			outfile << std::endl;
		}
	}
	outfile << "section .data" << std::endl;
	outfile << "\tArray_OOB: db \"Array index out of bounds!\",0xA,0" << std::endl;
	if (!initVars.empty() || !p.literals.empty()) {
		for (const auto& literal : p.literals) {
			outfile << "\t" << literal.mAlias << ": db \"";
			if (literal.mContent[literal.mContent.size() - 1] == '\n') {
				// Replace \n in string with ',0xA' put after the quote
				outfile << literal.mContent.substr(0, literal.mContent.size() - 1) << "\",0xA,0" << std::endl;
			} else {
				// Just write the string normally
				outfile << literal.mContent << "\"" << ",0" << std::endl;
			}
		}
		for (const auto& initVar : initVars) {
			addToSymbols(nullptr, initVar, initVar.mName, true);
			outfile << "\t" << initVar.mName << " " << getDefineBytes(initVar.mType.byteSize) << " ";
			for (int i = 0; i < initVar.mValues.size(); i++) {
				outfile << initVar.mValues[i]->mValue.mText;
				if (i != initVar.mValues.size() - 1)
					outfile << ", ";
			}
			outfile << std::endl;
		}
	}
	if (!otherVars.empty()) {
		outfile << "section .bss" << std::endl;
		for (const auto& otherVar : otherVars) {
			addToSymbols(nullptr, otherVar, otherVar.mName, true);
			if (otherVar.mType.builtinType != Builtin_Type::ARRAY)
				outfile << "\t" << otherVar.mName << " " << getReserveBytes(otherVar.mType.byteSize) << " " << otherVar.mValues.size() << std::endl;
			else
				outfile << "\t" << otherVar.mName << " " << getReserveBytes(otherVar.mType.subTypes[0].byteSize) << " " << otherVar.mType.byteSize /otherVar.mType.subTypes[0].byteSize << std::endl;
		}
	}
	outfile << std::endl;
	outfile << "section .text" << std::endl;

	std::vector<FuncCallStatement> external;
	for (const auto& funcCall : p.externalFunctions) {
		if (std::find(external.begin(), external.end(), funcCall) == external.end()) {
			outfile << "\textern ";
			if (!funcCall.mNamespace.empty())
				outfile << funcCall.mNamespace << "_";
			if (!funcCall.mClassName.empty())
				outfile << funcCall.mClassName << "_";
			outfile << funcCall.mFunctionName << std::endl;
			external.push_back(funcCall);
		}
	}

	setup(outfile);

	if (p.requires_libs) {
		printLibs(outfile);
	}
	// Loop over functions
	// Start with prologue 'push rbp', 'mov rbp, rsp'
	// For every variable, keep track of the offset
	// End with epilogue 'pop rbp'

	const char callingConvention[6][4] = {"di", "si", "d", "c", "8", "9"};

	for (const auto& klass : p.classes) {
		currentClass = klass.first;
		for (const auto& function : klass.second.mFunctions) {
			const auto& convention = ctx.getSymbolConvention(function.mName);
			if (convention != ctx.conventionsEnd) {
				if (((*convention).modifiers & Modifiers::PUBLIC) == Modifiers::PUBLIC) {
					outfile << "global " << klass.first << "_" << function.mName << std::endl;
				}
			}
			outfile << klass.first << "_" << function.mName << ":" << std::endl;
			outfile << "; =============== PROLOGUE ===============" << std::endl;
			outfile << "\tpush rbp" << std::endl;
			outfile << "\tmov rbp, rsp" << std::endl;
			outfile << "; =============== END PROLOGUE ===============" << std::endl;

			int offset = 0;
			int argOffset = 0;

			std::vector<std::string> localSymbols;

			const char* sizes[] = {"byte", "word", "dword", "qword"};
			int size = 0;
			for (size_t i = 0; i < function.mArgs.size(); i++) {
				const auto& arg = function.mArgs[i];
				size += arg.mType.byteSize;
			}
			size += 8;
			outfile << "\tsub rsp, " << nearestMultipleOf(size, 8) << std::endl;

			Type t = {"ref<>", Builtin_Type::REF, { Type {klass.second.mName, Builtin_Type::CLASS, {}, klass.second.mSize, klass.second.mSize }}, 8, 8};
			addToSymbols(&offset, Variable{t, klass.second.mName, {}});
			outfile << "\tmov qword [rbp" << offset << "], rdi" << std::endl;
			for (size_t i = 0; i < function.mArgs.size(); i++) {
				const auto& arg = function.mArgs[i];

				size_t incI = i + 1;
				int s = addToSymbols(&offset, Variable{arg.mType, arg.mName, {}});
				std::string reg = incI < 6 ? getRegister(callingConvention[incI], s) : "rbp+";
				outfile << "\tmov " << sizes[s] << " [rbp" << offset << "], " << reg << std::endl;
				localSymbols.push_back(arg.mName);
			}

			int allocs = 0;
			printBody(outfile, p, function.mBody, function.mName, &offset, &allocs);

			for (const auto& symbolName : localSymbols) {
				symbolTable.erase(symbolName);
			}

			outfile << ".exit:" << std::endl;
			outfile << "; =============== EPILOGUE ===============" << std::endl;
			outfile << "\tmov rsp, rbp" << std::endl;
			outfile << "\tpop rbp" << std::endl;
			outfile << "\tret" << std::endl;
			outfile << "; =============== END EPILOGUE ===============" << std::endl;
		}
	}
	for (const auto& function : p.functions) {
		if (function.mName == "main") {
			outfile << "\tglobal _start" << std::endl;
			outfile << std::endl << "_start:" << std::endl;
		} else {
			outfile << std::endl;
			const auto& convention = ctx.getSymbolConvention(function.mName);
			if (convention != ctx.conventionsEnd) {
				if (((*convention).modifiers & Modifiers::PUBLIC) == Modifiers::PUBLIC) {
					outfile << "global "  << function.mName << std::endl;
				}
			}
			outfile << function.mName << ":" << std::endl;
		}
		outfile << "; =============== PROLOGUE ===============" << std::endl;
		outfile << "\tpush rbp" << std::endl;
		outfile << "\tmov rbp, rsp" << std::endl;
		outfile << "; =============== END PROLOGUE ===============" << std::endl;


		// Construct symbol table, keeping track of scope
		// offset from stack, type
		int offset = 0;
		int argOffset = 0;

		std::vector<std::string> localSymbols;
		if (function.mName == "main") {
			const auto& argv = function.mArgs[0];
			argOffset = 15;
			addToSymbols(&argOffset, Variable {argv.mType, argv.mName, {} }, "rbp+");
			localSymbols.push_back(argv.mName);
		} else {
			int size = 0;
			for (size_t i = 0; i < function.mArgs.size(); i++) {
				const auto& arg = function.mArgs[i];
				size += arg.mType.byteSize;
			}
			outfile << "\tsub rsp, " << nearestMultipleOf(size, 8) << std::endl;
			const char* sizes[] = {"byte", "word", "dword", "qword"};
			for (size_t i = 0; i < function.mArgs.size(); i++) {
				const auto& arg = function.mArgs[i];

				int s = addToSymbols(&offset, Variable{arg.mType, arg.mName, {}});
				std::string reg = i < 6 ? getRegister(callingConvention[i], s) : "rbp+";
				outfile << "\tmov " << sizes[s] << " [rbp" << offset << "], " << reg << std::endl;
				localSymbols.push_back(arg.mName);
			}
		}

		int allocs = 0;
		printBody(outfile, p, function.mBody, function.mName, &offset, &allocs);

		for (const auto& symbolName : localSymbols) {
			symbolTable.erase(symbolName);
		}

		outfile << ".exit:" << std::endl;
		if (function.mName != "main") {
			outfile << "; =============== EPILOGUE ===============" << std::endl;
			outfile << "\tmov rsp, rbp" << std::endl;
			outfile << "\tpop rbp" << std::endl;
			outfile << "\tret" << std::endl;
			outfile << "; =============== END EPILOGUE ===============" << std::endl;
		} else {
			outfile << "\tmov rax, 60" << std::endl;;
			outfile << "\tsyscall" << std::endl;
		}
	}

	outfile.close();

	std::stringstream assembler;
	assembler << "yasm -f elf64";
	if (ctx.m_Configuration.m_BuildType == BuildType::DEBUG)
		assembler << " -g dwarf2";
	assembler << " -o " << buildPath.string() << "/" << fileName.stem().string() << ".o " << buildPath.string() << "/" << fileName.stem().string() << ".asm";
	//std::cout << assembler.str().c_str() << std::endl;
	std::system(assembler.str().c_str());
}

void X86_64LinuxYasmCompiler::setup(std::ofstream& outfile) {
	outfile << "array_out_of_bounds:" << std::endl;
	outfile << "\tmov rdi, 2" << std::endl;
	outfile << "\tmov rax, 1" << std::endl;
	outfile << "\tmov rsi, Array_OOB" << std::endl;
	outfile << "\tmov rdx, 28" << std::endl;
	outfile << "\tsyscall" << std::endl;
	outfile << "\tmov rdi, 1" << std::endl;
	outfile << "\tmov rax, 60" << std::endl;
	outfile << "\tsyscall" << std::endl;
	outfile << "global find_ui64_in_string" << std::endl;
	outfile << "find_ui64_in_string:" << std::endl;
	outfile << "\txor rcx, rcx" << std::endl;
	outfile << ".loop:" << std::endl;
	outfile << "\tcmp byte [rdi+rcx], 0x30" << std::endl;
	outfile << "\tjl .parse_number" << std::endl;
	outfile << "\tcmp byte [rdi+rcx], 0x39" << std::endl;
	outfile << "\tjg .parse_number" << std::endl;
	outfile << "\tmovzx rax, byte [rdi+rcx]" << std::endl;
	outfile << "\tpush rax" << std::endl;
	outfile << "\tinc rcx" << std::endl;
	outfile << "\tjmp .loop" << std::endl;
	outfile << ".parse_number:" << std::endl;
	outfile << "\txor r8, r8" << std::endl;
	outfile << "\txor rbx, rbx" << std::endl;
	outfile << "\tinc rbx" << std::endl;
	outfile << ".parse_loop:" << std::endl;
	outfile << "\tpop rax" << std::endl;
	outfile << "\tsub rax, 0x30" << std::endl;
	outfile << "\tmul rbx" << std::endl;
	outfile << "\tadd r8, rax" << std::endl;
	outfile << "\tmov rax, 10" << std::endl;
	outfile << "\tmul rbx" << std::endl;
	outfile << "\tmov rbx, rax" << std::endl;
	outfile << "\tloop .parse_loop" << std::endl;
	outfile << ".exit:" << std::endl;
	outfile << "\tmov rax, r8" << std::endl;
	outfile << "\tret" << std::endl;
}


void X86_64LinuxYasmCompiler::printLibs(std::ofstream& outfile) {
	outfile << "global print_ui64" << std::endl;
	outfile << "print_ui64:" << std::endl;
	outfile << "\tpush rbp" << std::endl;
	outfile << "\tmov rsi, rsp" << std::endl;
	outfile << "\tsub rsp, 22" << std::endl;
	outfile << "\tmov rax, rdi" << std::endl;
	outfile << "\tmov rbx, 0xA" << std::endl;
	outfile << "\txor rcx, rcx" << std::endl;
	outfile << "to_string_ui64:" << std::endl;
	outfile << "\tdec rsi" << std::endl;
	outfile << "\txor rdx, rdx" << std::endl;
	outfile << "\tdiv rbx" << std::endl;
	outfile << "\tadd rdx, 0x30 ; '0'" << std::endl;
	outfile << "\tmov byte [rsi], dl" << std::endl;
	outfile << "\tinc rcx" << std::endl;
	outfile << "\ttest rax, rax" << std::endl;
	outfile << "\tjnz to_string_ui64" << std::endl;
	outfile << ".write:" << std::endl;
	outfile << "\tinc rax" << std::endl;
	outfile << "\tmov rdi, 1" << std::endl;
	outfile << "\tmov rdx, rcx" << std::endl;
	outfile << "\tsyscall" << std::endl;
	outfile << "\tadd rsp, 22" << std::endl;
	outfile << "\tpop rbp" << std::endl;
	outfile << "\tret" << std::endl;
	outfile << "global print_ui64_newline" << std::endl;
	outfile << "print_ui64_newline:" << std::endl;
	outfile << "\tpush rbp" << std::endl;
	outfile << "\tmov rsi, rsp" << std::endl;
	outfile << "\tsub rsp, 22" << std::endl;
	outfile << "\tmov rax, rdi" << std::endl;
	outfile << "\tmov rbx, 0xA" << std::endl;
	outfile << "\txor rcx, rcx" << std::endl;
	outfile << "\tdec rsi" << std::endl;
	outfile << "\tmov byte [rsi], bl" << std::endl;
	outfile << "\tinc rcx" << std::endl;
	outfile << "\tjmp to_string_ui64" << std::endl;

	outfile << "global printString" << std::endl;
	outfile << "printString:" << std::endl;
	outfile << "\tmov rsi, rdi" << std::endl;
	outfile << "\txor rdx, rdx" << std::endl;
	outfile << ".strCountLoop:" << std::endl;
	outfile << "\tcmp byte [rdi], 0x0" << std::endl;
	outfile << "\tje .strCountDone" << std::endl;
	outfile << "\tinc rdx" << std::endl;
	outfile << "\tinc rdi" << std::endl;
	outfile << "\tjmp .strCountLoop" << std::endl;
	outfile << ".strCountDone:" << std::endl;
	outfile << "\tcmp rdx, 0" << std::endl;
	outfile << "\tje .prtDone" << std::endl;
	outfile << "\tmov rax, 1" << std::endl;
	outfile << "\tmov rdi, 1" << std::endl;
	outfile << "\tsyscall" << std::endl;
	outfile << ".prtDone:" << std::endl;
	outfile << "\tret" << std::endl;
}

void X86_64LinuxYasmCompiler::printFunctionCall(std::ofstream& outfile, const Programme& p, const FuncCallStatement& fc) {
	// TODO: We assume only one argument here
	// TODO: We assume entire expression tree is collapsed
	Expression* arg = fc.mArgs[0];
	const char* sizes[] = {"byte", "word", "dword", "qword"};
	if (arg->mValue.mType == TokenType::LITERAL && arg->mValue.mSubType == TokenSubType::STRING_LITERAL) {
		Literal l = p.findLiteralByContent(arg->mValue.mText).value();
		outfile << "; =============== FUNC CALL + STRING ===============" << std::endl;
		outfile << "\tmov rax, " << (fc.mFunctionName == "read" || fc.mFunctionName == "readln" ? 0 : 1) << std::endl;
		outfile << "\tmov rdi, " << (fc.mClassName == "stdin" ? 0 : 1) << std::endl;
		outfile << "\tmov rsi, " << l.mAlias << std::endl;
		outfile << "\tmov rdx, " << l.mSize << std::endl;
		outfile << "\tsyscall" << std::endl;
		outfile << "; =============== END FUNC CALL + STRING ===============" << std::endl;

	} else if (arg->mValue.mType == TokenType::LITERAL && arg->mValue.mSubType == TokenSubType::INTEGER_LITERAL) {
		outfile << "; =============== FUNC CALL + INT ===============" << std::endl;
		outfile << "\tmov rdi, " << arg->mValue.mText << std::endl;
		if (fc.mFunctionName == "write") {
			outfile << "\tcall print_ui64" << std::endl;
		} else if (fc.mFunctionName == "writeln") {
			outfile << "\tcall print_ui64_newline" << std::endl;
		}
		outfile << "; =============== END FUNC CALL + INT ===============" << std::endl;

	} else if (arg->mValue.mType == TokenType::IDENTIFIER) {
		if (!symbolTable.contains(arg->mValue.mText)) {
			// Class property
			const Class& klass = p.classes.at(currentClass);
			int propIndex = klass.getIndexOfProperty(arg->mValue.mText);
			int leftSize = getSizeFromType(klass.mFields[propIndex].mType);
			const char* moveAction = getMoveAction(3, leftSize, false);
			const char* reg = leftSize < 2 ? "rdi" : getRegister("di", leftSize);
			SymbolInfo& classInfo = symbolTable[currentClass];
			outfile << "\tmov r10, qword [rbp" << classInfo.offset << "]" << std::endl;
			outfile << "\t" << moveAction << " " << reg << ", " << sizes[leftSize] << " [r10+" << klass.mFields[propIndex].mOffset << "]" << "; variable CLASS " << klass.mName << "." << arg->mValue.mText << std::endl;
		} else {
			SymbolInfo& var = symbolTable[arg->mValue.mText];
			outfile << "; =============== FUNC CALL + VARIABLE ===============" << std::endl;
			if (var.type.builtinType == Builtin_Type::UI8) {
				outfile << "\tmov rax, 1" << std::endl;
				outfile << "\tmov rdi, 1" << std::endl;
				outfile << "\tlea rsi, " << var.location(true) << std::endl;
				outfile << "\tmov rdx, 1" << std::endl;
				outfile << "\tsyscall" << std::endl;
			} else {
				const char* moveAction = getMoveAction(3, var.size, false);
				const char* reg = var.size < 2 ? "rdi" : getRegister("di", var.size);
				outfile << "\t" << moveAction << " " << reg << ", " << sizes[var.size] << " " << var.location() << "; variable " << arg->mValue.mText << std::endl;
			}
		}
		if (fc.mFunctionName == "write") {
			outfile << "\tcall print_ui64" << std::endl;
		} else if (fc.mFunctionName == "writeln") {
			outfile << "\tcall print_ui64_newline" << std::endl;
		}

		outfile << "; =============== END FUNC CALL + VARIABLE ===============" << std::endl;

	} else if (arg->mValue.mType == TokenType::OPERATOR && arg->mValue.mText == "[") { // Array indexing
		SymbolInfo& var = symbolTable[arg->mChildren[0]->mValue.mText];
		std::string offset = arg->mChildren[1]->mValue.mText;
		outfile << "\tmov rdi, " << sizes[var.size] << " [" << var.reg;
		if (var.offset < 0)
			outfile << offset << "*" << (1 << var.size) ;
		else if (var.offset > 0)
			outfile << "+" << var.offset << "+" << offset << "*" << (1 << var.size) ;
		outfile << "]" << std::endl;
		//outfile << "\tmov rdi, qword [rsp+8+" << offset << "*8] ; Move the CLI arg into rdi" << std::endl;
		outfile << "\tcall printString" << std::endl;
		if (fc.mFunctionName == "writeln") {
			outfile << "\tmov rax, 1" << std::endl;
			outfile << "\tmov rdi, 1" << std::endl;
			outfile << "\tmov rsi, 0xA" << std::endl;
			outfile << "\tmov rdx, 1" << std::endl;
			outfile << "\tsyscall" << std::endl;
		}
	} else if (arg->mValue.mType == TokenType::OPERATOR && arg->mValue.mText == ".") { // Struct indexing
		outfile << "; We don't support struct indexing yet" << std::endl;
	}
}

void X86_64LinuxYasmCompiler::printSyscall(std::ofstream& outfile, const std::string& syscall) {
	if (syscallTable.contains(syscall)) {
		uint32_t call = syscallTable[syscall];
		outfile << "\tmov rax, " << call << std::endl;
	} else {
		std::string error = "Unexpected syscall '$', please expand syscall table";
		error.replace(error.find_first_of('$'), 1, syscall);
		throw std::runtime_error(error);
	}
}


void X86_64LinuxYasmCompiler::printBody(std::ofstream& outfile, const Programme& p, const Block& block, const std::string& labelName, int* offset, int* allocs) {
	const char callingConvention[6][4] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
	const char* sizes[] = {"byte", "word", "dword", "qword"};
	std::vector<std::string> localSymbols;
	int localOffset = 0;
	if (block.stackMemory != 0) {
		int toAlloc = nearestMultipleOf(block.stackMemory + block.biggestAlloc, 8);
		outfile << "\tsub rsp, " << toAlloc << std::endl;
		(*allocs) += toAlloc;
	}

	for (size_t i = 0; i < block.statements.size(); i++) {
		const auto& statement = block.statements[i];
		switch (statement.mType) {
			case Statement_Type::RETURN_CALL:
				printExpression(outfile, p, statement.mContent, 0);
				if (labelName == "main") {
					outfile << "\tmov rdi, rax" << std::endl;
				} else {
					if (*allocs > 0)
						outfile << "\tadd rsp, " << *allocs << std::endl;
					outfile << "\tjmp .exit" << std::endl;
				}

				break;
			case Statement_Type::VAR_DECLARATION:
				addToSymbols(offset, statement.variable.value());
				addToSymbols(&localOffset, statement.variable.value());
				localSymbols.push_back(statement.variable.value().mName);
				break;
			case Statement_Type::VAR_DECL_ASSIGN: {
				Variable v = statement.variable.value();
				if (v.mType.builtinType == Builtin_Type::ARRAY) {
					// Note: This subtraction is because we want to make the array initialise upwards towards the top of the stack
					// Reason for this is because register indexing is not allowed to go -rax, only +rax
					(*offset) -= int(v.mType.byteSize);
					localOffset -= int(v.mType.byteSize);
					addToSymbols(offset, v);
					addToSymbols(&localOffset, v);
					localSymbols.push_back(v.mName);

					SymbolInfo& arr = symbolTable[v.mName];
					int actualSize = getSizeFromByteSize(arr.type.subTypes[0].byteSize);

					for (int i = 0; i < v.mValues.size(); i++) {
						printExpression(outfile, p, v.mValues[i], 0);
						outfile << "\tmov " << sizes[actualSize] << " [" << arr.reg;
						if (arr.offset > 0)
							outfile << "+" << arr.offset - i * int(arr.type.byteSize / v.mValues.size());
						else if (arr.offset <= 0)
							outfile << "-" << -(arr.offset + i * int(arr.type.byteSize / v.mValues.size()));
						outfile << "], " << getRegister("a", actualSize) << "; VAR_DECL_ASSIGN ARRAY variable " << v.mName << "[" << i << "]" << std::endl;
					}
				} else if (v.mType.builtinType == Builtin_Type::STRUCT) {
					const Struct& s = p.structs.at(v.mType.name);
					(*offset) -= int(s.mSize);
					localOffset -= int(s.mSize);
					addToSymbols(offset, v);
					addToSymbols(&localOffset, v);
					localSymbols.push_back(v.mName);
					SymbolInfo& var = symbolTable[v.mName];
					if (v.mValues.size() == 1 && v.mValues[0]->mValue.mText == "@") {
						printExpression(outfile, p, v.mValues[0]->mChildren[0], 0);
						outfile << "\tmov r10, rax" << std::endl;
						for (int i = 0; i < s.mFields.size(); i++) {
							int actualSize = getSizeFromByteSize(s.mFields[i].mType.byteSize); // TODO: This won't work for nested structs
							const char* reg = getRegister("a", actualSize);
							outfile << "\tmov " << reg << ", " << sizes[actualSize] << " [r10+" << s.mFields[i].mOffset << "]" << std::endl;
							outfile << "\tmov " << sizes[actualSize] << " [" << var.reg;
							if (var.offset > 0)
								outfile << "+" << var.offset - s.mFields[i].mOffset;
							else
								outfile << "-" << -(var.offset + s.mFields[i].mOffset);
							outfile << "], " << getRegister("a", actualSize) << "; VAR_DECL_ASSIGN STRUCT " << v.mType.name << " " << v.mName << "." << s.mFields[i].mNames[0] << std::endl;
						}
					} else {
						for (int i = 0; i < s.mFields.size(); i++) {
							Expression* exp = v.mValues.at(i);
							if (exp == nullptr) continue;
							printExpression(outfile, p, exp, 0);
							int actualSize = getSizeFromByteSize(s.mFields[i].mType.byteSize); // TODO: This won't work for nested structs
							outfile << "\tmov " << sizes[actualSize] << " [" << var.reg;
							if (var.offset > 0)
								outfile << "+" << var.offset - s.mFields[i].mOffset;
							else
								outfile << "-" << -(var.offset + s.mFields[i].mOffset);
							outfile << "], " << getRegister("a", actualSize) << "; VAR_DECL_ASSIGN STRUCT " << v.mType.name << " " << v.mName << "." << s.mFields[i].mNames[0] << std::endl;
						}
					}
				} else if (v.mType.builtinType == Builtin_Type::CLASS) {
					const Class& c = p.classes.at(v.mType.name);
					(*offset) -= int(c.mSize);
					localOffset -= int(c.mSize);
					addToSymbols(offset, v);
					addToSymbols(&localOffset, v);
					localSymbols.push_back(v.mName);
					SymbolInfo& var = symbolTable[v.mName];

					if (v.mValues.size() == 1 && v.mValues[0]->mValue.mText == "@") {
						printExpression(outfile, p, v.mValues[0]->mChildren[0], 0);
						outfile << "\tmov r10, rax" << std::endl;
						for (int i = 0; i < c.mFields.size(); i++) {
							int actualSize = getSizeFromByteSize(c.mFields[i].mType.byteSize); // TODO: This won't work for nested structs
							const char* reg = getRegister("a", actualSize);
							outfile << "\tmov " << reg << ", " << sizes[actualSize] << " [r10+" << c.mFields[i].mOffset << "]" << std::endl;
							outfile << "\tmov " << sizes[actualSize] << " [" << var.reg;
							if (var.offset > 0)
								outfile << "+" << var.offset - c.mFields[i].mOffset;
							else
								outfile << "-" << -(var.offset + c.mFields[i].mOffset);
							outfile << "], " << getRegister("a", actualSize) << "; VAR_DECL_ASSIGN CLASS " << v.mType.name << " " << v.mName << "." << c.mFields[i].mNames[0] << std::endl;
						}
					} else {
						for (int i = 0; i < c.mFields.size(); i++) {
							Expression* exp = v.mValues.at(i);
							if (exp == nullptr) continue;
							printExpression(outfile, p, exp, 0);
							int actualSize = getSizeFromByteSize(c.mFields[i].mType.byteSize); // TODO: This won't work for nested structs
							outfile << "\tmov " << sizes[actualSize] << " [" << var.reg;
							if (var.offset > 0)
								outfile << "+" << var.offset - c.mFields[i].mOffset;
							else
								outfile << "-" << -(var.offset + c.mFields[i].mOffset);
							outfile << "], " << getRegister("a", actualSize) << "; VAR_DECL_ASSIGN CLASS " << v.mType.name << "." << c.mFields[i].mNames[0] << std::endl;

						}
					}
				} else {
					if (v.mValues.empty()) {
						std::cerr << "[X86_64 Compiler]: ERROR: Variable '" << v.mName << "' is being declared and assigned with no expression" << std::endl;
						exit(1);
					}
					printExpression(outfile, p, v.mValues[0], 0);
					int size = addToSymbols(offset, v);
					addToSymbols(&localOffset, v);
					localSymbols.push_back(v.mName);
					outfile << "\tmov " << sizes[size] << " " << symbolTable[v.mName].location() << ", " << getRegister("a", size) << "; VAR_DECL_ASSIGN else variable " << v.mName << std::endl;
				}

				break;
			}
			case Statement_Type::VAR_ASSIGNMENT: {
				Variable v = statement.variable.value();
				uint64_t index = v.mName.find('.');
				if (index != std::string::npos || statement.mContent != nullptr) {
					// Array or struct property
					if (v.mType.builtinType == Builtin_Type::ARRAY) {
						SymbolInfo& arr = symbolTable[v.mName];
						int actualSize = getSizeFromByteSize(arr.type.subTypes[0].byteSize);
						printExpression(outfile, p, statement.mContent, 0);
						outfile << "\tcmp rax, " << arr.type.byteSize / arr.type.subTypes[0].byteSize << "; check bounds" << std::endl;
						outfile << "\tjge array_out_of_bounds" << std::endl;
						outfile << "\tpush rax" << std::endl;

						if (v.mValues.empty()) {
							std::cerr << "[X86_64 Compiler]: ERROR: Array variable '" << v.mName << "' is being assigned with no expression" << std::endl;
							exit(1);
						}
						printExpression(outfile, p, v.mValues[0], 0);
						outfile << "\tpop r11" << std::endl;
						outfile << "\tmov " << sizes[actualSize] << " [" << arr.reg;
						if (arr.offset > 0)
							outfile << "+" << arr.offset << "+r11*" << int(arr.type.subTypes[0].byteSize);
						else if (arr.offset < 0)
							outfile << "-" << -arr.offset <<  "+r11*" << int(arr.type.subTypes[0].byteSize);
						else
							outfile << "+r11*" << int(arr.type.subTypes[0].byteSize);
						outfile << "], " << getRegister("a", actualSize) << "; VAR_ASSIGNMENT ARRAY " << v.mName << std::endl;

					} else if (v.mType.builtinType == Builtin_Type::REF) {
						// TODO: Allow for struct refs with property indexing
						SymbolInfo& ref = symbolTable[v.mName];
						int actualSize = getSizeFromByteSize(ref.type.subTypes[0].byteSize);
						printExpression(outfile, p, statement.mContent, 0);

						outfile << "\tlea r11, " << sizes[actualSize] << " [" << ref.reg;
						if (ref.offset > 0)
							outfile << "+" << ref.offset;
						else if (ref.offset < 0)
							outfile << "-" << -ref.offset;
						outfile << "]" << std::endl;
						outfile << "\tmov r11, qword [r11]" << std::endl;
						outfile << "\tmov rbx, " << int(ref.type.subTypes[0].byteSize) << std::endl;
						outfile << "\tmul rbx" << std::endl;
						outfile << "\tadd r11, rax" << std::endl;
						outfile << "\tpush r11" << std::endl;
						if (v.mValues.empty()) {
							std::cerr << "[X86_64 Compiler]: ERROR: Ref variable '" << v.mName << "' is being assigned with no expression" << std::endl;
							exit(1);
						}
						printExpression(outfile, p, v.mValues[0], 0);
						outfile << "\tpop r11" << std::endl;
						outfile << "\tmov " << sizes[actualSize] << " [r11], " << getRegister("a", actualSize) << "; VAR_ASSIGNMENT REF " << v.mName << std::endl;
					} else if (v.mType.builtinType == Builtin_Type::STRUCT) {
						std::string propName = v.mName.substr(index + 1);
						std::string varName = v.mName.substr(0, index);
						SymbolInfo& var = symbolTable[varName];
						const Struct& s = p.structs.at(v.mType.name);
						int fieldIndex = s.getIndexOfProperty(propName);
						const StructField& sf = s.mFields[fieldIndex];
						int actualSize = getSizeFromByteSize(sf.mType.byteSize);

						if (v.mValues.empty()) {
							std::cerr << "[X86_64 Compiler]: ERROR: Struct property '" << propName << "' of struct variable '" << varName << "' is being assigned with no expression" << std::endl;
							exit(1);
						}
						printExpression(outfile, p, v.mValues[0], 0);
						outfile << "\tmov " << sizes[actualSize] << " [" << var.reg;
						if (var.offset > 0)
							outfile << "+" << var.offset - sf.mOffset;
						else
							outfile << "-" << -(var.offset + sf.mOffset);
						outfile << "], " << getRegister("a", actualSize) << "; VAR_ASSIGNMENT STRUCT " << v.mName << std::endl;
					} else if (v.mType.builtinType == Builtin_Type::CLASS) {
						std::string propName = v.mName.substr(index + 1);
						std::string varName = v.mName.substr(0, index);
						SymbolInfo& var = symbolTable[varName];
						const Class& c = p.classes.at(v.mType.name);
						int fieldIndex = c.getIndexOfProperty(propName);
						const StructField& sf = c.mFields[fieldIndex];
						int actualSize = getSizeFromByteSize(sf.mType.byteSize);

						printExpression(outfile, p, v.mValues[0], 0);
						outfile << "\tmov " << sizes[actualSize] << " [" << var.reg;
						if (var.offset > 0)
							outfile << "+" << var.offset - sf.mOffset;
						else
							outfile << "-" << -(var.offset + sf.mOffset);
						outfile << "], " << getRegister("a", actualSize) << "; VAR_ASSIGNMENT CLASS " << v.mName << std::endl;
					}
				} else {
					// Redefinition
					if (v.mType.builtinType == Builtin_Type::ARRAY) {
						SymbolInfo& arr = symbolTable[v.mName];
						int actualSize = getSizeFromByteSize(arr.type.subTypes[0].byteSize);

						for (int i = 0; i < v.mValues.size(); i++) {
							printExpression(outfile, p, v.mValues[i], 0);
							outfile << "\tmov " << sizes[actualSize] << " [" << arr.reg;
							if (arr.offset > 0)
								outfile << "+" << arr.offset - i * int(arr.type.byteSize / v.mValues.size());
							else if (arr.offset <= 0)
								outfile << "-" << -(arr.offset + i * int(arr.type.byteSize / v.mValues.size()));
							outfile << "], " << getRegister("a", actualSize) << "; VAR_ASSIGNMENT ARRAY " << v.mName << "[" << i << "]" << std::endl;
						}
					} else if (v.mType.builtinType == Builtin_Type::STRUCT) {
						const Struct& s = p.structs.at(v.mType.name);
						SymbolInfo& var = symbolTable[v.mName];

						for (int i = 0; i < s.mFields.size(); i++) {
							Expression* exp = v.mValues.at(i);
							if (exp == nullptr) continue;
							printExpression(outfile, p, exp, 0);
							int actualSize = getSizeFromByteSize(s.mFields[i].mType.byteSize); // TODO: This won't work for nested structs
							outfile << "\tmov " << sizes[actualSize] << " [" << var.reg;
							if (var.offset > 0)
								outfile << "+" << var.offset - s.mFields[i].mOffset;
							else
								outfile << "-" << -(var.offset + s.mFields[i].mOffset);
							outfile << "], " << getRegister("a", actualSize) << "; VAR_ASSIGNMENT STRUCT " << v.mType.name << "." << s.mFields[i].mNames[0] << std::endl;
						}
					}  else if (v.mType.builtinType == Builtin_Type::CLASS) {
						const Class& c = p.classes.at(v.mType.name);
						SymbolInfo& var = symbolTable[v.mName];

						for (int i = 0; i < c.mFields.size(); i++) {
							Expression* exp = v.mValues.at(i);
							if (exp == nullptr) continue;
							printExpression(outfile, p, exp, 0);
							int actualSize = getSizeFromByteSize(c.mFields[i].mType.byteSize); // TODO: This won't work for nested structs
							outfile << "\tmov " << sizes[actualSize] << " [" << var.reg;
							if (var.offset > 0)
								outfile << "+" << var.offset - c.mFields[i].mOffset;
							else
								outfile << "-" << -(var.offset + c.mFields[i].mOffset);
							outfile << "], " << getRegister("a", actualSize) << "; VAR_ASSIGNMENT CLASS " << v.mType.name << "." << c.mFields[i].mNames[0] << std::endl;
						}
					} else {
						if (!symbolTable.contains(v.mName)) {
							// Class property
							if (v.mValues.empty()) {
								std::cerr << "[X86_64 Compiler]: ERROR: Class property '" << v.mName << "' is being assigned with no expression" << std::endl;
								exit(1);
							}
							printExpression(outfile, p, v.mValues[0], 0);
							const Class& klass = p.classes.at(currentClass);
							int propIndex = klass.getIndexOfProperty(v.mName);
							int leftSize = getSizeFromType(klass.mFields[propIndex].mType);
							SymbolInfo& classInfo = symbolTable[currentClass];
							outfile << "\tmov r10, qword [rbp" << classInfo.offset << "]" << std::endl;
							outfile << "\tmov " << sizes[leftSize] << " [r10+" << klass.mFields[propIndex].mOffset << "], " << getRegister("a", leftSize) << "; VAR_ASSIGNMENT else CLASS " << klass.mName << "." << v.mName << std::endl;
						} else {
							SymbolInfo& symbol = symbolTable[v.mName];
							if (v.mValues.empty()) {
								std::cerr << "[X86_64 Compiler]: ERROR: Variable '" << v.mName << "' is being assigned with no expression" << std::endl;
								exit(1);
							}
							printExpression(outfile, p, v.mValues[0], 0);
							bool dereferenceNeeded = symbol.reg == "rbp" || symbol.isGlobal;
							outfile << "\tmov " << sizes[symbol.size] << " " << symbol.location(dereferenceNeeded) << ", " << getRegister("a", symbol.size) << "; VAR_ASSIGNMENT else variable " << v.mName << std::endl;
						}
					}
				}
			}
			case Statement_Type::LOOP: {
				if (!statement.loopStatement.has_value()) continue;
				LoopStatement ls = statement.loopStatement.value();
				if (ls.mIterator.has_value()) {
					int size = addToSymbols(offset, ls.mIterator.value());
					addToSymbols(&localOffset, ls.mIterator.value());
					localSymbols.push_back(ls.mIterator.value().mName);
					SymbolInfo& symbol = symbolTable[ls.mIterator.value().mName];
					std::string op = "inc "; // TODO: Find better way of detecting whether to increment or decrement
					std::string label = ".label";
					uint32_t localLabelCount = ++labelCount;
					label = label.append(std::to_string(localLabelCount));
					loopLabels.push_back(label);
					printExpression(outfile, p, ls.mRange.value().mMinimum, 0);
					const char* reg = getRegister("a", size);
					outfile << "\tmov " << sizes[size] << " " << symbol.location() << ", " << reg << "; LOOP " << ls.mIterator.value().mName << std::endl;
					outfile << label << ":" << std::endl;
					printExpression(outfile, p, ls.mRange.value().mMaximum, 0);
					outfile << "\tcmp " << sizes[size] << " " << symbol.location() << ", " << reg << "; LOOP " << ls.mIterator.value().mName << std::endl;
					outfile << "\tjl .inside_label" << localLabelCount << std::endl;
					outfile << "\tjmp .not_label" << localLabelCount << std::endl;
					outfile << ".inside_label" << localLabelCount << ":" << std::endl;
					printBody(outfile, p, ls.mBody, label, offset, allocs);
					loopLabels.pop_back();
					outfile << ".skip_label" << localLabelCount << ":" << std::endl;
					outfile << "\tmov " << reg << ", " << sizes[size] << " "  << symbol.location() << "; LOOP " << ls.mIterator.value().mName << std::endl;
					outfile << "\t" << op << "rax" << std::endl;
					outfile << "\tmov " << sizes[size] << " " << symbol.location() << ", " << reg << "; LOOP " << ls.mIterator.value().mName << std::endl;
					outfile << "\tjmp .label" << localLabelCount << std::endl;
					outfile << ".not_label" << localLabelCount << ":" << std::endl;
					symbolTable.erase(ls.mIterator.value().mName);
				} else {
					if (statement.mContent == nullptr) {
						// We have "loop { ... }"
						std::string label = ".label";
						uint32_t localLabelCount = ++labelCount;
						label = label.append(std::to_string(localLabelCount));
						outfile << label << ":" << std::endl;
						loopLabels.push_back(label);
						printBody(outfile, p, ls.mBody, label, offset, allocs);
						loopLabels.pop_back();
						outfile << ".skip_label" << localLabelCount << ":" << std::endl;
						outfile << "\tjmp .label" << localLabelCount << std::endl;
						outfile << ".not_label" << localLabelCount << ":" << std::endl; // Used for break statements
					} else {
						// we have "until condition { ... }"
					}
				}
				break;
			}
			case Statement_Type::BREAK:
				if (!loopLabels.empty()) {
					outfile << "\tjmp .not_" << loopLabels.back().substr(1) << std::endl;
				}
				break;
			case Statement_Type::SKIP:
				if (!loopLabels.empty()) {
					outfile << "\tjmp .skip_" << loopLabels.back().substr(1) << std::endl;
				}
				break;
			case Statement_Type::FUNC_CALL: {
				FuncCallStatement fc = statement.funcCall.value();
				// If function is stdlib call, need to expand this into something better when stdlib expands
				if (fc.mIsRecursive) {
					outfile << "\tpush rdi" << std::endl;
					outfile << "\tpush rsi" << std::endl;
					outfile << "\tpush rdx" << std::endl;
					outfile << "\tpush rcx" << std::endl;
					outfile << "\tpush r8" << std::endl;
					outfile << "\tpush r9" << std::endl;
					outfile << "\tpush r10" << std::endl;
				}
				if (fc.mClassName == "stdout" || fc.mClassName == "stdin") {
					printFunctionCall(outfile, p, fc);
				} else {
					if (fc.mArgs.empty() && !fc.mClassName.empty()) {
						const SymbolInfo& symbol = symbolTable[fc.mClassName];
						if (symbol.type.builtinType == Builtin_Type::CLASS)
							outfile << "\tlea rax, " << symbol.location(true) << std::endl;
						else if ((symbol.type.builtinType == Builtin_Type::REF) && (symbol.type.subTypes[0].builtinType == Builtin_Type::CLASS)) {
							outfile << "\tmov rax, " << symbol.location(true) << std::endl;
						}
						outfile << "\tmov " << callingConvention[0] << ", rax" << std::endl;
					} else {
						uint8_t toChange = 0;
						if (!fc.mClassName.empty()) {
							const SymbolInfo& symbol = symbolTable[fc.mClassName];
							if (symbol.type.builtinType == Builtin_Type::CLASS) {
								outfile << "\tlea rax, " << symbol.location(true) << std::endl;
								outfile << "\tmov " << callingConvention[0] << ", rax" << std::endl;
							} else {
								outfile << "\tmov " << callingConvention[0] << ", " << sizes[symbol.size] << " " << symbol.location(true) << std::endl;
							}
							toChange = 1;
						}
						for (int i = toChange; i < fc.mArgs.size() + toChange; i++) {
							std::string value;
							Expression* expr = fc.mArgs[i - toChange];
							if (expr->mValue.mSubType == TokenSubType::STRING_LITERAL) {
								value = p.findLiteralByContent(expr->mValue.mText)->mAlias;
							} else {
								printExpression(outfile, p, expr, 0);
								value = "rax";
							}
							if (i <= 6)
								outfile << "\tmov " << callingConvention[i] << ", " << value << std::endl;
							else
								outfile << "\tpush " << value << std::endl; // These will be in wrong order
						}
					}
					if (fc.mFunctionName == "printf" && fc.mIsExternal) {
						outfile << "\tmov rax, 0" << std::endl;
					}
					if (fc.mFunctionName.substr(0, 4) == "SYS_") {
						printSyscall(outfile, fc.mFunctionName);
						outfile << "\tsyscall" << std::endl;
					} else if (fc.mFunctionName == "dealloc") {
						outfile << "\tmov rax, 11" << std::endl;
						outfile << "\tsyscall" << std::endl;
					} else {
						outfile << "\tcall ";
						if (!fc.mNamespace.empty())
							outfile << fc.mNamespace << "_";
						if (!fc.mClassName.empty()) {
							const SymbolInfo& symbol = symbolTable[fc.mClassName];
							if (symbol.type.builtinType == Builtin_Type::CLASS)
								outfile << symbol.type.name << "_";
							else {
								outfile << symbol.type.subTypes[0].name << "_";
							}
						}
						outfile << statement.funcCall.value().mFunctionName << std::endl;
					}
				}
				if (fc.mIsRecursive) {
					outfile << "\tpop r10" << std::endl;
					outfile << "\tpop r9" << std::endl;
					outfile << "\tpop r8" << std::endl;
					outfile << "\tpop rcx" << std::endl;
					outfile << "\tpop rdx" << std::endl;
					outfile << "\tpop rsi" << std::endl;
					outfile << "\tpop rdi" << std::endl;
				}
				break;
			}
			case Statement_Type::NOTHING:
				outfile << "\tnop" << std::endl;
				break;
			case Statement_Type::IF: {
				// TODO: If statements with a function call for expression don't work
				IfStatement is = statement.ifStatement.value();
				printExpression(outfile, p, statement.mContent, 0);
				std::string label = ".if";
				uint32_t localIfCount = ++ifCount;
				label = label.append(std::to_string(localIfCount));
				outfile << "\ttest rax, rax" << std::endl;
				outfile << "\tjnz " << label << std::endl;
				if (is.mElse.has_value())
					outfile << "\tjmp .else_if" <<  localIfCount << std::endl;
				else
					outfile << "\tjmp .end_if" << localIfCount << std::endl;
				outfile << label << ":" << std::endl;
				printBody(outfile, p, is.mBody, label, offset, allocs);
				outfile << "\tjmp .end_if" << localIfCount << std::endl;
				if (is.mElse.has_value()) {
					outfile << ".else_if" << localIfCount << ":" << std::endl;
					printBody(outfile, p, is.mElseBody.value(), label, offset, allocs);
				}
				outfile << ".end_if" << localIfCount << ":" << std::endl;
				break;
			}
			case Statement_Type::ARRAY_INDEX:
				outfile << "; Array indexing compiled. This shouldn't ever happen!" << std::endl;
				break;
		}
	}

	if (block.stackMemory != 0) {
		int toDeAlloc = nearestMultipleOf(block.stackMemory + block.biggestAlloc, 8);
		outfile << "\tadd rsp, " << toDeAlloc << std::endl;
		(*allocs) -= toDeAlloc;
	}

	*offset -= localOffset;
	for (const auto& symbolName : localSymbols) {
		symbolTable.erase(symbolName);
	}
}

std::stringstream X86_64LinuxYasmCompiler::moveToRegister(const std::string& reg, const SymbolInfo& symbol) {
	const char* sizes[] = {"byte", "word", "dword", "qword"};
	std::stringstream ss;
	switch (symbol.type.builtinType) {
		case Builtin_Type::UI8:
		case Builtin_Type::UI16:
			ss << "movzx ";
			break;
		case Builtin_Type::I8:
		case Builtin_Type::I16:
			ss << "movsx ";
			break;
		case Builtin_Type::I32:
			ss << "movsxd ";
			break;
		case Builtin_Type::UI32:
		case Builtin_Type::UI64:
		case Builtin_Type::I64:
		default:
			ss << "mov ";
			break;
	}
	ss << reg << ", " << sizes[symbol.size] << " " << symbol.location() << std::endl;
	return ss;
}

ExpressionPrinted X86_64LinuxYasmCompiler::printExpression(std::ofstream& outfile, const Programme& p, const Expression* expression, uint8_t nodeType) {
	if (expression == nullptr) return ExpressionPrinted{};

	const char* sizes[] = {"byte", "word", "dword", "qword"};

	if (nodeType == 0 && expression->mChildren.empty()) {
		// No expression, just base value
		if (expression->mValue.mSubType == TokenSubType::STRING_LITERAL) {
			outfile << "\tmov rax, " << p.findLiteralByContent(expression->mValue.mText).value().mAlias << std::endl;
		} else if (expression->mValue.mSubType == TokenSubType::INTEGER_LITERAL || expression->mValue.mSubType == TokenSubType::BOOLEAN_LITERAL) {
			outfile << "\tmov rax, " << expression->mValue.mText << std::endl;
		} else if (expression->mValue.mSubType == TokenSubType::CHAR_LITERAL) {
			outfile << "\tmov rax, " << int(expression->mValue.mText[0]) << "; CHAR_LITERAL '" << escape(&expression->mValue.mText[0]) << "'" << std::endl;
		} else if (expression->mValue.mType == TokenType::IDENTIFIER) {
			if (!symbolTable.contains(expression->mValue.mText)) {
				// Class member
				const Class& klass = p.classes.at(currentClass);
				int propIndex = klass.getIndexOfProperty(expression->mValue.mText);
				bool sign = klass.mFields[propIndex].mType.name[0] == 'i';
				int size = getSizeFromType(klass.mFields[propIndex].mType);
				const char* reg = size < 2 || sign ? "rax" : getRegister("a", size);
				const char* moveAction = getMoveAction(3, size, sign);
				SymbolInfo& classInfo = symbolTable[currentClass];
				outfile << "\tmov r10, qword [rbp" << classInfo.offset << "]" << std::endl;
				outfile << "\t" << moveAction << " " << reg << ", " << sizes[size] << " [r10+" << klass.mFields[propIndex].mOffset << "]" << "; printExpression, left identifier, class " << currentClass << "." << expression->mValue.mText << std::endl;
			} else {
				SymbolInfo& val = symbolTable[expression->mValue.mText];
				bool sign = val.type.name[0] == 'i'; // This might cause a problem later with user-defined types starting with i
				const char* moveAction = getMoveAction(3, val.size, sign);
				const char* reg = (val.size < 2 || sign) ? "rax" : getRegister("a", val.size);
				if (val.isGlobal) {
					outfile << "\t" << moveAction << " " << reg << ", " << sizes[val.size] << " "  << val.location(true) << "; printExpression global variable " << expression->mValue.mText << std::endl;
				} else {
					bool dereferenceNeeded = val.reg == "rbp";
					outfile << "\t" << moveAction << " " << reg << ", " << sizes[val.size] << " " << val.location(dereferenceNeeded) << "; printExpression variable " << expression->mValue.mText << std::endl;
				}
			}
		} else if (expression->mValue.mType == TokenType::OPERATOR) {
			if (expression->mValue.mText == "++") {
				outfile << "\tinc rax" << std::endl;
			} else if (expression->mValue.mText == "--") {
				outfile << "\tdec rax" << std::endl;
			}
		}
	}
	if (expression->mChildren.empty())
		return ExpressionPrinted{};

	if (expression->mChildren[0] == nullptr) {
		return ExpressionPrinted{};
	}
	if (expression->mValue.mType != TokenType::OPERATOR) {
		return ExpressionPrinted{};
	}
	if (expression->mChildren[0]->mValue.mSubType == TokenSubType::STRING_LITERAL) {
		return ExpressionPrinted{};
	}

	ExpressionPrinted curr = {};
	int leftSize;
	bool leftSign = false;
	int rightSize;
	bool rightSign = false;

	if (expression->mValue.mText == "[") {
		// blep = arr[i]
		//         [
		//      arr  i
		printExpression(outfile, p, expression->mChildren[1], 0);

		if (!symbolTable.contains(expression->mChildren[0]->mValue.mText)) {
			std::cerr << "Unknown symbol '" << expression->mChildren[0]->mValue.mText << "' at printExpression" << std::endl;
			throw std::runtime_error("Unknown symbol");
		}
		SymbolInfo& arr = symbolTable[expression->mChildren[0]->mValue.mText];
		// NOTE: Isn't only arrays, but can also be refs, strings, or if we want, numbers indexed to the bits
		int actualSize = getSizeFromByteSize(arr.type.subTypes[0].byteSize);
		if (arr.type.builtinType == Builtin_Type::ARRAY) {
			if (arr.offset > 0) {
				outfile << "\tcmp rax, " << sizes[actualSize] << " " << arr.location() << "; check bounds" << std::endl;
			} else {
				outfile << "\tcmp rax, " << arr.type.byteSize / arr.type.subTypes[0].byteSize << "; check bounds" << std::endl;
			}
			outfile << "\tjge array_out_of_bounds" << std::endl;
			bool sign = arr.type.subTypes[0].name[0] == 'i'; // This might cause a problem later with user-defined types starting with i
			const char* moveAction = getMoveAction(3, actualSize, sign);
			const char* reg = actualSize < 2 ? "r12" : getRegister("12", actualSize);
			outfile << "\t" << moveAction << " " << reg << ", " << sizes[actualSize] << " [" << arr.reg;
			if (arr.offset > 0)
				outfile << "+" << arr.offset << "+rax*" << int(arr.type.subTypes[0].byteSize);
			else if (arr.offset < 0)
				outfile << "-" << -arr.offset << "+rax*" << int(arr.type.subTypes[0].byteSize);
			else
				outfile << "+rax*" << int(arr.type.subTypes[0].byteSize);
			outfile << "]" << "; printExpression array " << expression->mChildren[0]->mValue.mText << std::endl;
			if (nodeType == 1) {
				outfile << "\tmov rbx, r12; printExpression, nodeType=1, array index" << std::endl;
			} else {
				outfile << "\tmov rax, r12" << std::endl;
			}
		} else if (arr.type.builtinType == Builtin_Type::REF) {
			const char* moveAction = getMoveAction(3, actualSize, false);
			bool dereferenceNeeded = arr.reg == "rbp";
			outfile << "\tmov rbx, " << arr.type.subTypes[0].byteSize << std::endl;
			outfile << "\tmul rbx" << std::endl;
			outfile << "\tmov rbx, qword " << arr.location(dereferenceNeeded) << std::endl;
			outfile << "\tadd rax, rbx" << std::endl;
			const char* reg = actualSize < 2 ? "r11" : getRegister("11", actualSize);
			outfile << "\t" << moveAction << " " << reg << ", " <<  sizes[actualSize] << " [rax]" << "; printExpression ref " << expression->mChildren[0]->mValue.mText << std::endl;
			if (nodeType == 1) {
				outfile << "\tmov rbx, r11; printExpression, nodeType=1, ref index" << std::endl;
			} else {
				outfile << "\tmov rax, r11" << std::endl;
			}
		}
		return ExpressionPrinted{true, false, 3};
	} else if (expression->mValue.mText == "(") {
		const char callingConvention[6][4] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
		std::stringstream ss;
		bool printArgs = false;
		int i = 0;
		std::string classVariable = "";
		outfile << "\tpush rdi" << std::endl;
		outfile << "\tpush rsi" << std::endl;
		outfile << "\tpush rdx" << std::endl;
		outfile << "\tpush rcx" << std::endl;
		outfile << "\tpush r8" << std::endl;
		outfile << "\tpush r9" << std::endl;
		outfile << "\tpush r10" << std::endl;
		for (const auto& child : expression->mChildren) {
			if (child->mValue.mText == "e" || child->mValue.mText == ":") continue;
			if (child->mValue.mText == "(") {
				printArgs = true;
				if (!classVariable.empty()) {
					const SymbolInfo& symbol = symbolTable[classVariable];
					if (symbol.type.builtinType == Builtin_Type::CLASS)
						outfile << "\tlea rax, " << symbol.location(true) << std::endl;
					else if ((symbol.type.builtinType == Builtin_Type::REF) && (symbol.type.subTypes[0].builtinType == Builtin_Type::CLASS)) {
						outfile << "\tmov rax, " << symbol.location(true) << std::endl;
					}
					outfile << "\tmov " << callingConvention[0] << ", rax" << std::endl;
					i++;
				}
				continue;
			}
			if (!printArgs) {
				if (child->mValue.mType == TokenType::OPERATOR)
					ss << "_";
				else {
					if (!symbolTable.contains(child->mValue.mText)) {
						ss << child->mValue.mText;
						continue;
					}
					const SymbolInfo& symbol = symbolTable[child->mValue.mText];
					if ((symbol.type.builtinType == Builtin_Type::REF) && (symbol.type.subTypes[0].builtinType == Builtin_Type::CLASS)) {
						ss << symbol.type.subTypes[0].name;
					} else {
						ss << symbol.type.name;
					}
					classVariable = child->mValue.mText;
				}
			} else {
				std::string value;

				if (child->mValue.mSubType == TokenSubType::STRING_LITERAL) {
					value = p.findLiteralByContent(child->mValue.mText)->mAlias;
				} else {
					printExpression(outfile, p, child, 0);
					value = "rax";
				}
				if (i <= 6)
					outfile << "\tmov " << callingConvention[i] << ", " << value << std::endl;
				else
					outfile << "\tpush " << value << std::endl; // TODO: This is a bug, values should be pushed onto the stack in reverse order
				i++;
			}
		}
		if (ss.str() == "printf") {
			outfile << "\tmov rax, 0" << std::endl;
		}
		if (ss.str().substr(0, 4) == "SYS_") {
			printSyscall(outfile, ss.str());
			outfile << "\tsyscall" << std::endl;
		} else if (ss.str() == "alloc") {
			outfile << "\tmov rax, 9" << std::endl;
			outfile << "\tmov rsi, rdi" << std::endl;
			outfile << "\txor rdi, rdi" << std::endl;
			outfile << "\tmov rdx, 3" << std::endl;
			outfile << "\tmov r10, 34" << std::endl;
			outfile << "\txor r8, r8" << std::endl;
			outfile << "\txor r9, r9" << std::endl;
			outfile << "\tsyscall" << std::endl;
		} else {
			outfile << "\tcall " << ss.str() << std::endl;
		}
		if (nodeType == 1) {
			outfile << "\tmov rbx, rax; printExpression, nodeType=1, function call" << std::endl;
		}

		outfile << "\tpop r10" << std::endl;
		outfile << "\tpop r9" << std::endl;
		outfile << "\tpop r8" << std::endl;
		outfile << "\tpop rcx" << std::endl;
		outfile << "\tpop rdx" << std::endl;
		outfile << "\tpop rsi" << std::endl;
		outfile << "\tpop rdi" << std::endl;

		return ExpressionPrinted{ true, false, 3 };
	} else if (expression->mValue.mSubType == TokenSubType::OP_UNARY) {
		if (expression->mValue.mText == "\\") {
			Expression* child = expression->mChildren[0];
			if (child->mValue.mType == TokenType::IDENTIFIER) {
				SymbolInfo& left = symbolTable[expression->mChildren[0]->mValue.mText];
				leftSize = left.size;
				outfile << "\tlea rax, " << left.location(true) << "; printExpression variable " << expression->mChildren[0]->mValue.mText << std::endl;
			} else if (child->mValue.mSubType == TokenSubType::STRING_LITERAL) {
				outfile << "\tlea rax, " << p.findLiteralByContent(child->mValue.mText).value().mAlias << std::endl;
			} else if (child->mValue.mType == TokenType::LITERAL) {
				throw std::runtime_error("Cannot get reference of a literal value");
			} else {
				throw std::runtime_error("Unexpected getting a reference");
			}
		} else if (expression->mValue.mText == "@") {
			ExpressionPrinted valPrinted = printExpression(outfile, p, expression->mChildren[0], -1);
			if (!valPrinted.printed) {
				Expression* child = expression->mChildren[0];
				if (child->mValue.mType == TokenType::IDENTIFIER) {
					SymbolInfo& left = symbolTable[expression->mChildren[0]->mValue.mText];
					leftSign = left.type.name[0] == 'i'; // This might cause a problem later with user-defined types starting with i
					leftSize = left.size;
					const char* reg = "r11";
					const char* moveAction = getMoveAction(3, leftSize, leftSign);
					outfile << "\t" << moveAction << " " << reg << ", " << sizes[left.size] << " " << left.location() << "; printExpression variable " << expression->mChildren[0]->mValue.mText << std::endl;
					outfile << "\tmov rax, [r11]" << std::endl;
				} else if (child->mValue.mSubType == TokenSubType::STRING_LITERAL) {
					throw std::runtime_error("Cannot dereference a string literal value");
				} else if (child->mValue.mType == TokenType::LITERAL) {
					outfile << "\tmov r11, " << child->mValue.mText << std::endl;
					outfile << "\tmov rax, [r11]" << std::endl;
				} else {
					throw std::runtime_error("Unexpected dereference");
				}
			} else {
				outfile << "\tmov rax, [rax]" << std::endl;
			}
		} else if (expression->mValue.mText == "!") {
			ExpressionPrinted valPrinted = printExpression(outfile, p, expression->mChildren[0], -1);
			if (valPrinted.printed) {
				// NOTE: If we knew the type that is in rax, we could possibly only do a xor, which is a bit more efficient
				outfile << "\tcmp rax, 0" << std::endl;
				outfile << "\tsete al" << std::endl;
				outfile << "\tmovzx rax, al" << std::endl;
			} else {
				Expression* child = expression->mChildren[0];
				if (child->mValue.mType == TokenType::IDENTIFIER) {
					SymbolInfo& left = symbolTable[expression->mChildren[0]->mValue.mText];
					leftSign = left.type.name[0] == 'i'; // This might cause a problem later with user-defined types starting with i
					leftSize = left.size;
					if (left.type.builtinType == Builtin_Type::BOOL) {
						const char* reg = leftSize < 2 ? "rax" : getRegister("a", leftSize);
						const char* moveAction = getMoveAction(3, leftSize, leftSign);
						outfile << "\t" << moveAction << " " << reg << ", " << sizes[left.size] << " " << left.location() << "; printExpression !" << expression->mChildren[0]->mValue.mText << std::endl;
						outfile << "\txor rax, 1" << std::endl;
					} else {
						outfile << "\tcmp " << sizes[left.size] << " " << left.location() << ", 0; printExpression !" << expression->mChildren[0]->mValue.mText << std::endl;
						outfile << "\tsete al" << std::endl;
						outfile << "\tmovzx rax, al" << std::endl;
					}
				} else if (child->mValue.mSubType == TokenSubType::STRING_LITERAL) {
					throw std::runtime_error("Cannot `!` a string literal value");
				} else if (child->mValue.mType == TokenType::LITERAL) {
					outfile << "\tmov rax, " << child->mValue.mText << std::endl;
					if (child->mValue.mSubType == TokenSubType::BOOLEAN_LITERAL)
						outfile << "\txor rax, 1" << std::endl;
					// Else, flip all the bits. If someone wants to use ints as bools, they will have to `!intVal & 1` instead of `!intVal`.
					else {
						outfile << "\tcmp rax, 0" << std::endl;
						outfile << "\tsete al" << std::endl;
						outfile << "\tmovzx rax, al" << std::endl;
					}
				} else {
					throw std::runtime_error("Unexpected `!`");
				}
			}
		} else if (expression->mValue.mText == "~") {
			ExpressionPrinted valPrinted = printExpression(outfile, p, expression->mChildren[0], -1);
			if (valPrinted.printed)
				outfile << "\tnot rax" << std::endl;
			else {
				Expression* child = expression->mChildren[0];
				if (child->mValue.mType == TokenType::IDENTIFIER) {
					SymbolInfo& left = symbolTable[expression->mChildren[0]->mValue.mText];
					leftSign = left.type.name[0] == 'i'; // This might cause a problem later with user-defined types starting with i
					leftSize = left.size;
					const char* reg = "rax";
					const char* moveAction = getMoveAction(3, leftSize, leftSign);
					outfile << "\t" << moveAction << " " << reg << ", " << sizes[left.size] << " " << left.location() << "; printExpression ~" << expression->mChildren[0]->mValue.mText << std::endl;
					outfile << "\tnot rax" << std::endl;
				} else if (child->mValue.mSubType == TokenSubType::STRING_LITERAL) {
					throw std::runtime_error("Cannot `~` a string literal value");
				} else if (child->mValue.mType == TokenType::LITERAL) {
					if (child->mValue.mSubType == TokenSubType::CHAR_LITERAL) {
						outfile << "\tmov rax, " << int(child->mValue.mText[0]) << std::endl;
					} else {
						outfile << "\tmov rax, " << child->mValue.mText << std::endl;
					}
					outfile << "\tnot rax" << std::endl;
				} else {
					throw std::runtime_error("Unexpected `~`");
				}
			}
		} else if (expression->mValue.mText == "-") {
			ExpressionPrinted valPrinted = printExpression(outfile, p, expression->mChildren[0], -1);
			if (valPrinted.printed)
				outfile << "\tneg rax" << std::endl;
			else {
				Expression* child = expression->mChildren[0];
				if (child->mValue.mType == TokenType::IDENTIFIER) {
					SymbolInfo& left = symbolTable[expression->mChildren[0]->mValue.mText];
					leftSign = left.type.name[0] == 'i'; // This might cause a problem later with user-defined types starting with i
					leftSize = left.size;
					const char* reg = "rax";
					const char* moveAction = getMoveAction(3, leftSize, leftSign);
					outfile << "\t" << moveAction << " " << reg << ", " << sizes[left.size] << " " << left.location() << "; printExpression -" << expression->mChildren[0]->mValue.mText << std::endl;
					outfile << "\tneg rax" << std::endl;
				} else if (child->mValue.mSubType == TokenSubType::STRING_LITERAL) {
					throw std::runtime_error("Cannot `-` a string literal value");
				} else if (child->mValue.mType == TokenType::LITERAL) {
					if (child->mValue.mSubType == TokenSubType::CHAR_LITERAL) {
						outfile << "\tmov rax, " << int(child->mValue.mText[0]) << std::endl;
					} else {
						outfile << "\tmov rax, " << child->mValue.mText << std::endl;
					}
					outfile << "\tneg rax" << std::endl;
				} else {
					throw std::runtime_error("Unexpected `-`");
				}
			}
		}
		if (nodeType == 1) {
			outfile << "\tmov rbx, rax; printExpression, nodeType=1, unary op" << std::endl;
		}
		return ExpressionPrinted{ true, false, 3 };
	} else if (expression->mValue.mText == ".") {
		// We have a struct property
		if (!symbolTable.contains(expression->mChildren[0]->mValue.mText)) {
			throw std::runtime_error("Trying to get property of unknown symbol " + expression->mChildren[0]->mValue.mText);
		}
		SymbolInfo& left = symbolTable[expression->mChildren[0]->mValue.mText]; // Left is variable name
		std::string propName = expression->mChildren[1]->mValue.mText; // Right is property name
		std::string structName = left.type.name;
		// Left can be a ref<T> with subtype struct
		if (!left.type.subTypes.empty()) {
			structName = left.type.subTypes[0].name; // TODO: We assume only one level deep
		}
		int actualSize = 0;
		if (left.type.builtinType == Builtin_Type::STRUCT) {
			const Struct& s = p.structs.at(structName);
			int fieldIndex = s.getIndexOfProperty(propName);
			const StructField& sf = s.mFields[fieldIndex];
			actualSize = getSizeFromByteSize(sf.mType.byteSize);
			const char* moveAction = getMoveAction(3, actualSize, sf.mType.name[0] == 'i');

			outfile << "\t" << moveAction << " " << getRegister("a", actualSize >= 2 ? actualSize : 3) << ", " << sizes[actualSize] << " [" << left.reg;
			if (left.reg != "rbp") {
				outfile << "+" << left.offset + sf.mOffset;
			} else {
				if (left.offset > 0)
					outfile << "+" << int(left.offset - sf.mOffset);
				else
					outfile << "-" << -(int(left.offset + sf.mOffset));
			}
			outfile << "]; printExpression struct " << structName << "." << propName << std::endl;
			if (nodeType == 1) {
				outfile << "\tmov rbx, rax; printExpression, nodeType=1, struct property" << std::endl;
			}
		} else {
			// Class
			const Class& c = p.classes.at(structName);
			int fieldIndex = c.getIndexOfProperty(propName);
			const StructField& sf = c.mFields[fieldIndex];
			actualSize = getSizeFromByteSize(sf.mType.byteSize);
			const char* moveAction = getMoveAction(3, actualSize, sf.mType.name[0] == 'i');

			outfile << "\t" << moveAction << " " << getRegister("a", actualSize >= 2 ? actualSize : 3) << ", " << sizes[actualSize] << " [" << left.reg;
			if (left.reg != "rbp") {
				outfile << "+" << left.offset + sf.mOffset;
			} else {
				if (left.offset > 0)
					outfile << "+" << int(left.offset - sf.mOffset);
				else
					outfile << "-" << -(int(left.offset + sf.mOffset));
			}
			outfile << "]; printExpression class " << structName << "." << propName << std::endl;
			if (nodeType == 1) {
				outfile << "\tmov rbx, rax; printExpression, nodeType=1, class property" << std::endl;
			}
		}
		return ExpressionPrinted{ true, false, actualSize };
	}

	ExpressionPrinted leftPrinted = printExpression(outfile, p, expression->mChildren[0], -1);
	if (leftPrinted.printed) {
		outfile << "\tpush rax; printExpression, leftPrinted, save left" << std::endl; // Save left
	}
	ExpressionPrinted rightPrinted = printExpression(outfile, p, expression->mChildren[1], 1);

	if (expression->mChildren[0]->mValue.mType == TokenType::IDENTIFIER) {
		if (!symbolTable.contains(expression->mChildren[0]->mValue.mText)) {
			// Class member
			const Class& klass = p.classes.at(currentClass);
			int propIndex = klass.getIndexOfProperty(expression->mChildren[0]->mValue.mText);
			leftSign = klass.mFields[propIndex].mType.name[0] == 'i';
			leftSize = getSizeFromType(klass.mFields[propIndex].mType);
			const char* reg = leftSize < 2 || leftSign ? "rax" : getRegister("a", leftSize);
			const char* moveAction = getMoveAction(3, leftSize, leftSign);
			SymbolInfo& classInfo = symbolTable[currentClass];
			outfile << "\tmov r10, qword [rbp" << classInfo.offset << "]" << std::endl;
			outfile << "\t" << moveAction << " " << reg << ", " << sizes[leftSize] << " [r10+" << klass.mFields[propIndex].mOffset << "]" << "; printExpression, left identifier, class " << currentClass << "." << expression->mChildren[0]->mValue.mText << std::endl;
		} else {
			SymbolInfo& left = symbolTable[expression->mChildren[0]->mValue.mText];
			leftSign = left.type.name[0] == 'i'; // This might cause a problem later with user-defined types starting with i
			leftSize = left.size;
			const char* reg = leftSize < 2 || leftSign ? "rax" : getRegister("a", leftSize);
			const char* moveAction = getMoveAction(3, leftSize, leftSign);
			bool dereferenceNeeded = left.isGlobal;
			if (left.reg == "rbp")
				outfile << "\t" << moveAction << " " << reg << ", " << sizes[left.size] << " " << left.location() << "; printExpression, left identifier, rbp variable " << expression->mChildren[0]->mValue.mText << std::endl;
			else
				outfile << "\t" << moveAction << " " << reg << ", " << sizes[left.size] << " " << left.location(dereferenceNeeded) << "; printExpression, left identifier, not rbp" << std::endl;
		}
	} else if (expression->mChildren[0]->mValue.mSubType == TokenSubType::INTEGER_LITERAL) {
		int size = getSizeFromNumber(expression->mChildren[0]->mValue.mText);
		if (size < 0) {
			leftSign = true;
			size = -size - 1;
		}
		leftSize = size;
		const char* reg = "rax";//getRegister("a", size);
		outfile << "\tmov " << reg << ", " << expression->mChildren[0]->mValue.mText << "; printExpression, left int" << std::endl;
	} else if (expression->mChildren[0]->mValue.mSubType == TokenSubType::CHAR_LITERAL) {
		leftSign = false;
		leftSize = 1;
		outfile << "\tmov rax, " << int(expression->mChildren[0]->mValue.mText[0]) << "; printExpression, left char '" << escape(&expression->mChildren[0]->mValue.mText[0]) << "'" << std::endl;
	} else if (expression->mChildren[0]->mValue.mSubType == TokenSubType::BOOLEAN_LITERAL) {
		leftSign = false;
		leftSize = 1;
		outfile << "\tmov rax, " << expression->mChildren[0]->mValue.mText << "; printExpression, left bool" << std::endl;
	} else {
		leftSize = leftPrinted.size;
	}

	if (expression->mChildren[1]->mValue.mType == TokenType::IDENTIFIER) {
		if (!symbolTable.contains(expression->mChildren[1]->mValue.mText)) {
			const Class& klass = p.classes.at(currentClass);
			int propIndex = klass.getIndexOfProperty(expression->mChildren[1]->mValue.mText);
			rightSign = klass.mFields[propIndex].mType.name[0] == 'i';
			rightSize = getSizeFromType(klass.mFields[propIndex].mType);
			const char* reg = rightSize < 2 || rightSign ? "rbx" : getRegister("b", rightSize);
			const char* moveAction = getMoveAction(3, rightSize, rightSign);
			SymbolInfo& classInfo = symbolTable[currentClass];
			outfile << "\tmov r10, qword [rbp" << classInfo.offset << "]" << std::endl;
			outfile << "\t" << moveAction << " " << reg << ", " << sizes[rightSize] << " [r10+" << klass.mFields[propIndex].mOffset << "]" << "; printExpression, right identifier, class " << currentClass << "." << expression->mChildren[1]->mValue.mText << std::endl;
		} else {
			SymbolInfo& right = symbolTable[expression->mChildren[1]->mValue.mText];
			rightSign = right.type.name[0] == 'i'; // This might cause a problem later with user-defined types starting with i
			rightSize = right.size;
			const char* reg = rightSize < 2 || rightSign ? "rbx" : getRegister("b", rightSize);
			const char* moveAction = getMoveAction(3, rightSize, rightSign);
			bool dereferenceNeeded = right.isGlobal;
			if (right.reg == "rbp")
				outfile << "\t" << moveAction << " " << reg << ", " << sizes[right.size] << " " << right.location() << "; printExpression, right identifier, rbp variable " << expression->mChildren[1]->mValue.mText << std::endl;
			else
				outfile << "\t" << moveAction << " " << reg << ", " << sizes[right.size] << " " << right.location(dereferenceNeeded) << "; printExpression, right identifier, not rbp" << std::endl;
		}
	} else if (expression->mChildren[1]->mValue.mSubType == TokenSubType::INTEGER_LITERAL) {
		int size = getSizeFromNumber(expression->mChildren[1]->mValue.mText);
		if (size < 0) {
			rightSize = true;
			size = -size - 1;
		}
		rightSize = size;
		const char* reg = "rbx";//getRegister("b", size);
		outfile << "\tmov " << reg << ", " << expression->mChildren[1]->mValue.mText << "; printExpression, right int" << std::endl;
	} else if (expression->mChildren[1]->mValue.mSubType == TokenSubType::CHAR_LITERAL) {
		rightSign = false;
		rightSize = 1;
		outfile << "\tmov rbx, " << int(expression->mChildren[1]->mValue.mText[0]) << "; printExpression, right char '" << escape(&expression->mChildren[1]->mValue.mText[0]) << "'" << std::endl;
	} else if (expression->mChildren[1]->mValue.mSubType == TokenSubType::BOOLEAN_LITERAL) {
		rightSign = false;
		rightSize = 1;
		outfile << "\tmov rbx, " << expression->mChildren[1]->mValue.mText << "; printExpression, right bool" << std::endl;
	} else {
		rightSize = rightPrinted.size;
	}

	curr.sign = leftPrinted.sign || rightPrinted.sign || leftSign || rightSign;

	if (leftPrinted.printed) {
		outfile << "\tpop rax; printExpression, leftPrinted, recover left" << std::endl; // Recover left
	}

	if (expression->mValue.mText == "+") {
		int size = getEvenSize(leftSize, rightSize);
		std::string r1 = getRegister("a", size);
		std::string r2 = getRegister("b", size);
		outfile << "\tadd " << r1 << ", " << r2 << std::endl;
	} else if (expression->mValue.mText == "-") {
		int size = getEvenSize(leftSize, rightSize);
		if (size == 3)
			outfile << "\tsub rax, rbx" << std::endl;
		else {
			std::string r1 = getRegister("a", size+1);
			std::string r2 = getRegister("b", size+1);
			outfile << "\tsub " << r1 << ", " << r2 << std::endl;
		}
	} else if (expression->mValue.mText == "*") {
		int size = getEvenSize(leftSize, rightSize) + 1;
		if (size > 3) size = 3;
		std::string r2 = getRegister("b", size);
		if (curr.sign)
			outfile << "\timul " << sizes[size] << " " << r2 << std::endl;
		else
			outfile << "\tmul " << sizes[size] << " " << r2 << std::endl;
	} else if (expression->mValue.mText == "/") {
		int size = getEvenSize(leftSize, rightSize);
		std::string r1 = getRegister("a", size);
		std::string r2 = getRegister("b", size);
		outfile << "\t" << convertARegSize(size) << std::endl;
		outfile << "\txor rdx, rdx; Clearing rdx for division" << std::endl;
		if (curr.sign)
			outfile << "\tidiv " << r1 << ", " << r2 << std::endl;
		else
			outfile << "\tdiv " << r1 << ", " << r2 << std::endl;
	} else if (expression->mValue.mText == "%") {
		int size = getEvenSize(leftSize, rightSize);
		std::string r1 = getRegister("a", size);
		std::string r2 = getRegister("b", size);
		std::string r3 = size > 0 ? getRegister("d", size) : "ah";
		outfile << "\t" << convertARegSize(size) << std::endl;
		outfile << "\txor rdx, rdx; Clearing rdx for division" << std::endl;
		if (curr.sign)
			outfile << "\tidiv " << r1 << ", " << r2 << std::endl;
		else
			outfile << "\tdiv " << r1 << ", " << r2 << std::endl;
		outfile << "\tmov " << r1 << ", " << r3 << std::endl; // Remainder of div (mod) is stored in d-register
	} else if (expression->mValue.mText == "<") {
		printConditionalMove(outfile, leftSize, rightSize, "cmovl");
	} else if (expression->mValue.mText == "<=") {
		printConditionalMove(outfile, leftSize, rightSize, "cmovle");
	} else if (expression->mValue.mText == "==") {
		printConditionalMove(outfile, leftSize, rightSize, "cmove");
	} else if (expression->mValue.mText == "!=") {
		printConditionalMove(outfile, leftSize, rightSize, "cmovne");
	} else if (expression->mValue.mText == ">") {
		printConditionalMove(outfile, leftSize, rightSize, "cmovg");
	} else if (expression->mValue.mText == ">=") {
		printConditionalMove(outfile, leftSize, rightSize, "cmovge");
	} else if (expression->mValue.mText == "&") {
		int size = getEvenSize(leftSize, rightSize);
		std::string r1 = getRegister("a", size);
		std::string r2 = getRegister("b", size);
		outfile << "\tand " << r1 << ", " << r2 << std::endl;
	} else if (expression->mValue.mText == "|") {
		int size = getEvenSize(leftSize, rightSize);
		std::string r1 = getRegister("a", size);
		std::string r2 = getRegister("b", size);
		outfile << "\tor " << r1 << ", " << r2 << std::endl;
	} else if (expression->mValue.mText == "^") {
		int size = getEvenSize(leftSize, rightSize);
		std::string r1 = getRegister("a", size);
		std::string r2 = getRegister("b", size);
		outfile << "\txor " << r1 << ", " << r2 << std::endl;
	} else if (expression->mValue.mText == ">>") {
		std::string r2 = getRegister("c", 0);
		outfile << "\tmov rcx, rbx; printExpression, shift right" << std::endl;
		outfile << "\tshr rax" << ", " << r2 << std::endl;
	} else if (expression->mValue.mText == "<<") {
		std::string r2 = getRegister("c", 0);
		outfile << "\tmov rcx, rbx; printExpression, shift left" << std::endl;
		outfile << "\tshl rax" << ", " << r2 << std::endl;
	}

	if (nodeType == 1) {
		std::string r1 = getRegister("b", leftSize);
		std::string r2 = getRegister("a", rightSize);
		outfile << "\tmov rbx, rax; printExpression, nodeType=1" << std::endl;
	}

	curr.size = getEvenSize(leftSize, rightSize);
	curr.printed = true;

	return curr;
}

void X86_64LinuxYasmCompiler::printConditionalMove(std::ofstream& outfile, int leftSize, int rightSize, const char* instruction) {
	int size = getEvenSize(leftSize, rightSize);
	std::string r1 = getRegister("a", size);
	std::string r2 = getRegister("b", size);
	outfile << "\tmov rcx, 0" << std::endl;
	outfile << "\tmov rdx, 1" << std::endl;
	outfile << "\tcmp " << r1 << ", " << r2 << std::endl;
	outfile << "\t" << instruction << " rcx, rdx" << std::endl;
	outfile << "\tmov rax, rcx; printConditionalMove" << std::endl;
}

const char* X86_64LinuxYasmCompiler::getRegister(const std::string& reg, int size) {
	if (size == 0) {
		if (reg == "a") return "al";
		if (reg == "b") return "bl";
		if (reg == "c") return "cl";
		if (reg == "d") return "dl";
		if (reg == "si") return "sil";
		if (reg == "di") return "dil";
		if (reg == "bp") return "bpl";
		if (reg == "sp") return "spl";
		if (reg == "8") return "r8b";
		if (reg == "9") return "r9b";
		if (reg == "10") return "r10b";
		if (reg == "11") return "r11b";
		if (reg == "12") return "r12b";
		if (reg == "13") return "r13b";
		if (reg == "14") return "r14b";
		if (reg == "15") return "r15b";
	} else if (size == 1) {
		if (reg == "a") return "ax";
		if (reg == "b") return "bx";
		if (reg == "c") return "cx";
		if (reg == "d") return "dx";
		if (reg == "si") return "si";
		if (reg == "di") return "di";
		if (reg == "bp") return "bp";
		if (reg == "sp") return "sp";
		if (reg == "8") return "r8w";
		if (reg == "9") return "r9w";
		if (reg == "10") return "r10w";
		if (reg == "11") return "r11w";
		if (reg == "12") return "r12w";
		if (reg == "13") return "r13w";
		if (reg == "14") return "r14w";
		if (reg == "15") return "r15w";
	} else if (size == 2) {
		if (reg == "a") return "eax";
		if (reg == "b") return "ebx";
		if (reg == "c") return "ecx";
		if (reg == "d") return "edx";
		if (reg == "si") return "esi";
		if (reg == "di") return "edi";
		if (reg == "bp") return "ebp";
		if (reg == "sp") return "esp";
		if (reg == "8") return "r8d";
		if (reg == "9") return "r9d";
		if (reg == "10") return "r10d";
		if (reg == "11") return "r11d";
		if (reg == "12") return "r12d";
		if (reg == "13") return "r13d";
		if (reg == "14") return "r14d";
		if (reg == "15") return "r15d";
	} else if (size == 3) {
		if (reg == "a") return "rax";
		if (reg == "b") return "rbx";
		if (reg == "c") return "rcx";
		if (reg == "d") return "rdx";
		if (reg == "si") return "rsi";
		if (reg == "di") return "rdi";
		if (reg == "bp") return "rbp";
		if (reg == "sp") return "rsp";
		if (reg == "8") return "r8";
		if (reg == "9") return "r9";
		if (reg == "10") return "r10";
		if (reg == "11") return "r11";
		if (reg == "12") return "r12";
		if (reg == "13") return "r13";
		if (reg == "14") return "r14";
		if (reg == "15") return "r15";
	}
	return "UNREACHABLE";
}

int X86_64LinuxYasmCompiler::getSizeFromNumber(const std::string& text) {
	int64_t value = atol(text.c_str());
	if (value < 0) {
		// Has to be signed
		if (value >= -128) {
			return -1;
		} else if (value >= -32768) {
			return -2;
		} else if (value >= -2147483648) {
			return -3;
		} else {
			return -4;
		}
	} else {
		if (value <= 255) {
			return 0;
		} else if (value <= 65535) {
			return 1;
		} else if (value <= 4294967295) {
			return 2;
		} else {
			return 3;
		}
	}
}

int X86_64LinuxYasmCompiler::getEvenSize(int size1, int size2) {
	if (size1 == 0 || size2 > size1) return size2;
	else return size1;
}

const char* X86_64LinuxYasmCompiler::getMoveAction(int regSize, int valSize, bool isSigned) {
	if (regSize - valSize >= 2) { // rax with byte, word, but not dword or qword
		if (isSigned) return "movsx";
		else return "movzx";
	} else {
		if (isSigned && valSize == 2) return "movsxd";
		else return "mov";
	}
}

int X86_64LinuxYasmCompiler::getSizeFromByteSize(size_t byteSize) {
	switch (byteSize) {
		case 1: return 0;
		case 2: return 1;
		case 4: return 2;
		case 8: return 3;
		default: return 3;
	}
}

int X86_64LinuxYasmCompiler::getSizeFromType(const Type& type) {
	switch (type.builtinType) {
		case Builtin_Type::UNDEFINED:
		case Builtin_Type::VOID:
		case Builtin_Type::STRUCT:
		case Builtin_Type::ARRAY:
		case Builtin_Type::CLASS:
			break;
		case Builtin_Type::UI8:
		case Builtin_Type::I8:
		case Builtin_Type::F8:
		case Builtin_Type::CHAR:
		case Builtin_Type::BOOL:
			return 0;
		case Builtin_Type::UI16:
		case Builtin_Type::I16:
		case Builtin_Type::F16:
			return 1;
		case Builtin_Type::UI32:
		case Builtin_Type::I32:
		case Builtin_Type::F32:
			return 2;
		case Builtin_Type::UI64:
		case Builtin_Type::I64:
		case Builtin_Type::F64:
		case Builtin_Type::REF:
			return 3;
	}
	if (type.name == "string")
		return 3;
	return 0;
}

int X86_64LinuxYasmCompiler::nearestMultipleOf(int toRound, int multiple) {
	return toRound + (multiple - (toRound % multiple)) % multiple;
}

const char* X86_64LinuxYasmCompiler::convertARegSize(int size) {
	if (size == 0) return "cbw";
	if (size == 1) return "cwd";
	if (size == 2) return "cdq";
	if (size == 3) return "cqo";
	return "UNREACHABLE";
}

const char* X86_64LinuxYasmCompiler::getDefineBytes(size_t byteSize) {
	switch (byteSize) {
		case 1: return "db";
		case 2: return "dw";
		case 4: return "dd";
		case 8: return "dq";
		case 16: return "ddq";
	}
	return "UNREACHABLE";
}

const char* X86_64LinuxYasmCompiler::getReserveBytes(size_t byteSize) {
	switch (byteSize) {
		case 1: return "resb";
		case 2: return "resw";
		case 4: return "resd";
		case 8: return "resq";
		case 16: return "resdq";
	}
	return "UNREACHABLE";
}

const char* X86_64LinuxYasmCompiler::escape(const char* input) {
	switch (*input) {
		case '\a': return "\\a"; break;
		case '\b': return "\\b"; break;
		case '\f': return "\\f"; break;
		case '\n': return "\\n"; break;
		case '\r': return "\\r"; break;
		case '\t': return "\\t"; break;
		case '\v': return "\\v"; break;
		case '\\': return "\\\\"; break;
		case '\'': return "\\'"; break;
		case '\"': return "\\\""; break;
		case '\?': return "\\\?"; break;
		default: return input;
	}
}
