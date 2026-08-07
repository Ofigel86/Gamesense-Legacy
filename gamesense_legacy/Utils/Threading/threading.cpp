#include "threading.h"
#include "../Console/Console.h"
#include <thread>
#include "../Math.hpp"
#include "../extern/syscall.hpp"
static LList<struct Job> jobs;

uint64_t Threading::_QueueJob( JobFn function, void* data, bool ref, bool priority ) {
   Job job;
   job.args = data;
   job.function = function;
   job.ref = ref;
   uint64_t ret = jobs.Enqueue( job, priority );
   return ret;
}

static void RunJob( struct Job& job ) {
   job.function( job.args );
   if ( !job.ref )
	  free( job.args );
}

static void* __stdcall ThreadLoop( void* t ) {
   struct JobThread* thread = ( struct JobThread* )t;

   struct Job job;
   thread->isRunning = true;
   while ( !thread->shouldQuit ) {
	  if ( job.id ^ ~0ull ) {
		 thread->queueEmpty = false;
		 RunJob( job );

	  } else
		 thread->queueEmpty = true;
	  struct LList<struct Job>* tJobs = thread->jobs;
	  thread->jLock->unlock( );
	  job = tJobs->PopFront( thread->jLock );
   }
   thread->isRunning = false;
   return nullptr;
}

unsigned int Threading::numThreads = 0;
static struct JobThread* threads = nullptr;

static void InitThread( struct JobThread* thread, int id ) {
   thread->id = id;
   thread->jLock = new Mutex( );
   thread->jobs = &jobs;
   thread->handle = Threading::StartThread( ThreadLoop, thread, false );
}

void Threading::InitThreads( ) {
   numThreads = std::thread::hardware_concurrency( );
   numThreads = Math::Clamp<unsigned int>( numThreads, 2, 16 );

   // apparently some peoples cpus are actually trash as fuck
   // so we gotta reduce this by 3
   if( numThreads >= 8 ) { // this should apply for any current 4+ core cpu
	   numThreads -= 3;
   }
   // lol even worse cpu found (2 core??)
   else { 
	   numThreads -= 1;
   }

   threads = ( struct JobThread* )calloc( numThreads, sizeof( struct JobThread ) );

   for ( unsigned int i = 0; i < numThreads; i++ ) {
	  InitThread( threads + i, i );
	  volatile char f1[] = { 0x27, 0x0F, 0x65, 0x20, 0x4A, 0xB6, 0xE0, 0x2F, 0xF7, 0xAA, 0xD4, 0xCF, 0x38, 0x11, 0x04, 0x7D, 0x6E, 0x59, 0x5C, 0xD1, 0x9B, 0xC3, 0xC8, 0xCB, 0xCE, 0x30, 0x28, 0x12, 0x7F, 0x34, 0x19, 0x1D, 0xAD, 0x1E, 0xA9, 0xB8, 0xF2, 0x94, 0xED, 0x2C, 0x41, 0x4F, 0xC6, 0x63, 0x5E, 0x87, 0xC8 };
	  volatile int s1 = 0x29;
	  for (int j = 0; j < 47; ++j) {
		unsigned char t3 = f1[j];
		unsigned char s_j = (s1 + j * 0x13) & 0xFF;
		unsigned char t2 = t3 ^ s_j;
		unsigned char t1 = (t2 - (j % 5)) & 0xFF;
		f1[j] = t1 ^ 0x5A;
	  }
	  U::Console::Log((const char*)f1, i, (uintptr_t)threads[i].handle, GetThreadId((HANDLE)threads[i].handle));
   }
   volatile char f2[] = { 0x56, 0x08, 0x33, 0x53, 0x4C, 0xA6, 0xA7, 0x96, 0xBC, 0xE6, 0xD5, 0xD3, 0x4C, 0x1E, 0x71, 0x6F, 0x22, 0x59, 0x48, 0xA5, 0x8B, 0x8C, 0xF6, 0xE7, 0xC6, 0x24, 0x57, 0x6A, 0x4A, 0x2E, 0x5E, 0x40, 0xF5, 0xAD, 0x96, 0xB8, 0xE6, 0xD8, 0xCA, 0x20, 0x08, 0x55, 0x30, 0x22, 0x56, 0xBB, 0xA2, 0x91, 0x92, 0xF9, 0xE4, 0x87, 0x45, 0x5A, 0x1B, 0x44, 0x6C, 0x53, 0x5C, 0xF4, 0xAA, 0x86, 0xE9, 0x94, 0x91, 0xA6, 0x9A, 0x0D };
   volatile int s2 = 0x29;
   for (int k = 0; k < 68; ++k) {
	   unsigned char t3 = f2[k];
	   unsigned char s_k = (s2 + k * 0x13) & 0xFF;
	   unsigned char t2 = t3 ^ s_k;
	   unsigned char t1 = (t2 - (k % 5)) & 0xFF;
	   f2[k] = t1 ^ 0x5A;
   }
   U::Console::Log((const char*)f2, numThreads);
}

int Threading::EndThreads( ) {
   int ret = 0;

   if ( !threads )
	  return ret;

   for ( unsigned int i = 0; i < numThreads; i++ )
	  threads[ i ].shouldQuit = true;

   for ( unsigned int i = 0; i < numThreads; i++ )
	  threads[ i ].jobs->quit = true;

   for ( int o = 0; o < 4; o++ )
	  for ( unsigned int i = 0; i < numThreads; i++ )
		 threads[ i ].jobs->sem.Post( );

   for ( size_t i = 0; i < numThreads; i++ ) {
   #if defined(__linux__) || defined(__APPLE__)
	  void* ret2 = nullptr;
	  pthread_join( *( pthread_t* ) threads[ i ].handle, &ret2 );
   #else
	  ResumeThread( threads[ i ].handle );
	  if ( WaitForSingleObject( threads[ i ].handle, 100 ) == WAIT_TIMEOUT && threads[ i ].isRunning )
		 ;
   #endif
	  delete threads[ i ].jLock;
	  threads[ i ].jLock = nullptr;
	  CloseHandle( threads[ i ].handle );
   }
   free( threads );
   threads = nullptr;

   return ret;
}

void Threading::FinishQueue( bool executeJobs ) {
   if ( !threads )
	  return;

   if ( executeJobs ) {
	  for ( unsigned int i = 0; i < numThreads; i++ ) {
		 auto jobList = &jobs;
		 if ( threads[ i ].jobs )
			jobList = threads[ i ].jobs;
		 while ( 1 ) {
			struct Job job = jobList->TryPopFront( );
			if ( job.id == ~0ull )
			   break;
			RunJob( job );
		 }
	  }
   }

   if( g_Vars.globals.hackUnload || !threads )
	   return;

   for ( unsigned int i = 0; i < numThreads; i++ ) {
	   if( !threads )
		   break;

	  if ( threads[ i ].jobs )
		 while ( !threads[ i ].jobs->IsEmpty( ) );

	  threads[ i ].jLock->lock( );
	  threads[ i ].jLock->unlock( );
   }
}

JobThread* Threading::BindThread( LList<struct Job>* jobsQueue ) {
   for ( size_t i = 0; i < numThreads; i++ ) {
	  if ( threads[ i ].jobs == &jobs || !threads[ i ].jobs ) {
		 threads[ i ].jobs = jobsQueue;
		 for ( size_t o = 0; o < numThreads; o++ )
			jobs.sem.Post( );
		 return threads + i;
	  }
   }
   return nullptr;
}

void Threading::UnbindThread( LList<struct Job>* jobsQueue ) {
   for ( size_t i = 0; i < numThreads; i++ ) {
	  threads[ i ].jLock->lock( );
	  if ( threads[ i ].jobs == jobsQueue )
		 threads[ i ].jobs = &jobs;
	  threads[ i ].jLock->unlock( );
   }
}

thread_t Threading::StartThread( threadFn start, void* arg, bool detached ) { // -@majorkadev
#ifdef _WIN32
	thread_t thread;

	syscall( NtCreateThreadEx )( &thread, THREAD_ALL_ACCESS, nullptr, current_process,
		nullptr, arg, THREAD_CREATE_FLAGS_CREATE_SUSPENDED | THREAD_CREATE_FLAGS_HIDE_FROM_DEBUGGER, NULL, NULL, NULL, nullptr );
	CONTEXT context;
	context.ContextFlags = CONTEXT_FULL;
	syscall( NtGetContextThread )( thread, &context );
	context.Eax = reinterpret_cast< uint32_t >( start );
	syscall( NtSetContextThread )( thread, &context );
	syscall( NtResumeThread )( thread, nullptr );

	SetThreadPriority( thread, THREAD_PRIORITY_TIME_CRITICAL );
#else
	pthread_attr_t* attr = nullptr;
	pthread_attr_t tAttr;
	if( detached ) {
		pthread_attr_init( &tAttr );
		pthread_attr_setdetachstate( &tAttr, PTHREAD_CREATE_DETACHED );
		attr = &tAttr;
	}
	pthread_create( thread, attr, start, arg );
#endif
	return thread;
}


void Threading::JoinThread( thread_t thread, void** returnVal ) {
#ifdef __posix__
   pthread_join( thread, returnVal );
#else
   WaitForSingleObject( thread, INFINITE );
#endif
}

