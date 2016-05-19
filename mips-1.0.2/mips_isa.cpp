/**
 * @file      mips_isa.cpp
 * @author    Sandro Rigo
 *            Marcus Bartholomeu
 *            Alexandro Baldassin (acasm information)
 *
 *            The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:50:52 -0300
 * 
 * @brief     The ArchC i8051 functional model.
 * 
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

#include  "mips_isa.H"
#include  "mips_isa_init.cpp"
#include  "mips_bhv_macros.H"


//If you want debug information for this model, uncomment next line
#define DEBUG_MODEL
#include "ac_debug_model.H"


//!User defined macros to reference registers.
#define Ra 31
#define Sp 29

// 'using namespace' statement to allow access to all
// mips-specific datatypes
using namespace mips_parms;

static int processors_started = 0;
#define DEFAULT_STACK_SIZE (256*1024)



/**#######################################################################**/
/**#######################################################################**/
/**#######################################################################**/

enum MyInsType { common, load, store , jump, branch, undefined };
struct MyInstruction {
  MyInsType type;      // Hazard de dados ocorrem apenas em instrucoes de load e write, portanto precisamos guardar o tipo da instrucao do pipeline
  int r1;
  int r2;
  int r3;
}; typedef struct MyInstruction MyInstruction;

MyInstruction null_instruction = { .type = undefined, .r1 = 0, .r2 = 0, .r3 = 0 };
int bubble = 0;

#define PIPELINE_SIZE 5

//TODO: Change this when wanted
#define IS_SUPERESCALAR
#ifdef IS_SUPERESCALAR
	#define SUPERESCALAR_SIZE 2
#else
	#define SUPERESCALAR_SIZE 1
#endif

MyInstruction pipeline[SUPERESCALAR_SIZE][PIPELINE_SIZE];

//// Use PIPELINE(x) como se fosse o vetor pipeline
int current_pipe_index;
#define PIPELINE(x) pipeline[current_pipe_index][x]
#define NON_CURRENT_PIPELINE(x) pipeline[!current_pipe_index][x]

#define FIRST_PIPE(x) pipeline[0][x]
#define SECOND_PIPE(x) pipeline[1][x]

#define IS_PIPE_FILLED current_pipe_index == 0

#define BUBBLE newInstruction(undefined, 0, 0, 0)

MyInstruction newInstruction(MyInsType type, int r1, int r2, int r3)
{
  MyInstruction ins;
  ins.type = type;
  ins.r1 = r1;
  ins.r2 = r2;
  ins.r3 = r3;
  return ins;
}

const char *getStr(MyInsType t){
  if(t == common)
    return "common";
  if(t == load)
    return "load";
  if(t == store)
    return "store";
  if(t == jump)
    return "jump";
  if(t == branch)
    return "branch";
  if(t == undefined)
    return "undefined";
}

void setPipeline(MyInsType type, int r1, int r2, int r3)
{
  PIPELINE(0).type = type;
  PIPELINE(0).r1 = r1;
  PIPELINE(0).r2 = r2;
  PIPELINE(0).r3 = r3;
}

//Mascara que indica quais registradores nao terao seu valor atualizado
//a tempo para a instrucao que acabou de entrar no fetch utilizar
void get_non_updated_mask(char* mask){
	mask[0] = -1;/*
	mask[29] = -1;
	mask[30] = -1;
	mask[31] = -1;*/
	for(int i = 1; i < 32; i++) mask[i] = 0;
	
	if(FIRST_PIPE(2).type == load && mask[ FIRST_PIPE(2).r1 ] != -1) {
		mask[ FIRST_PIPE(2).r1 ] = 1;
	}
	if(SECOND_PIPE(2).type == load && mask[ SECOND_PIPE(2).r1] != -1) {
		mask[ SECOND_PIPE(2).r1 ] = 1;
	}
}

void checkhazards(){
  #ifdef IS_SUPERESCALAR		
  
  // Checar somente se existem instruções para fetch
  if(FIRST_PIPE(0).type == undefined || SECOND_PIPE(0).type == undefined){
	return;
  }
  
  //TODO: This is the cheetos 
  do{
	  
	dbg_printf("<<<<<<<<<<<<<<<<<<<BEFORE CHECK HAZARDS>>>>>>>>>>>>>>>>>>>\n");
	dbg_printf("   ---- FIRST_PIPE \n");
		for(int i = 0; i < PIPELINE_SIZE; i++)
		dbg_printf("%s %d %d %d\n", getStr(FIRST_PIPE(i).type), FIRST_PIPE(i).r1, FIRST_PIPE(i).r2, FIRST_PIPE(i).r3);
	dbg_printf("   ---- SECOND_PIPE \n");
		for(int i = 0; i < PIPELINE_SIZE; i++)
		dbg_printf("%s %d %d %d\n", getStr(SECOND_PIPE(i).type), SECOND_PIPE(i).r1, SECOND_PIPE(i).r2, SECOND_PIPE(i).r3);
	dbg_printf("   ---- \n");
	
	//Deslocando pipeline temporalmente a partir do segundo estagio
	for (int i = PIPELINE_SIZE-1; i >= 2; i--){
		FIRST_PIPE(i) = FIRST_PIPE(i-1);
		SECOND_PIPE(i) = SECOND_PIPE(i-1);
        }
    
        char non_updated[32];
        get_non_updated_mask(non_updated);
    
	FIRST_PIPE(1) = SECOND_PIPE(1) = BUBBLE;
	//Se ambos registradores r2 e r3 nao terao seus valores atualizados a tempo
	if( (non_updated[ NON_CURRENT_PIPELINE(0).r2 ] == 1 || non_updated[ NON_CURRENT_PIPELINE(0).r3] == 1) /**RAW com problemas de load, assim como no processador escalar**/){
		dbg_printf("BUBBLE DETECTED on %d\n", (!current_pipe_index) + 1);
		bubble++;
		if(NON_CURRENT_PIPELINE(0).type == load)
			non_updated[ NON_CURRENT_PIPELINE(0).r1 ] = 1;
	} else {
		NON_CURRENT_PIPELINE(1) = NON_CURRENT_PIPELINE(0);
		NON_CURRENT_PIPELINE(0) = BUBBLE;
		if(NON_CURRENT_PIPELINE(1).type == load)
			non_updated[ NON_CURRENT_PIPELINE(1).r1 ] = 1;
	}
	//TODO: The error is here
	if((non_updated[ PIPELINE(0).r2 ] == 1 || non_updated[ PIPELINE(0).r3] == 1)  || // RAW com problemas de load
	   ( NON_CURRENT_PIPELINE(0).r1 != 0 && (NON_CURRENT_PIPELINE(0).r1 == PIPELINE(0).r2 || NON_CURRENT_PIPELINE(0).r1 == PIPELINE(0).r3)) || // RAW
	   ( PIPELINE(0).r1 != 0 && PIPELINE(0).r1 == NON_CURRENT_PIPELINE(0).r1) || // WAW
	   ( PIPELINE(0).r1 != 0 && (PIPELINE(0).r1 == NON_CURRENT_PIPELINE(0).r2 || PIPELINE(0).r1 == NON_CURRENT_PIPELINE(0).r3 ))  ) { // WAR
		dbg_printf("BUBBLE DETECTED on %d -- current\n", (current_pipe_index+1));
		bubble++;
	} else {
		PIPELINE(1) = PIPELINE(0);
		PIPELINE(0) = BUBBLE;
	}

	

	dbg_printf("<<<<<<<<<<<<<<<<<<<AFTER CHECK HAZARDS>>>>>>>>>>>>>>>>>>>\n");
	/*dbg_printf("  Mask :  --- [ ");
	for (int i = 0; i < 32; i++)
		dbg_printf("{%d, %d} ", i, non_updated[i]);
	dbg_printf("]\n");
	*/
	dbg_printf("   ---- FIRST_PIPE \n");
		for(int i = 0; i < PIPELINE_SIZE; i++)
		dbg_printf("%s %d %d %d\n", getStr(FIRST_PIPE(i).type), FIRST_PIPE(i).r1, FIRST_PIPE(i).r2, FIRST_PIPE(i).r3);
	dbg_printf("   ---- SECOND_PIPE \n");
		for(int i = 0; i < PIPELINE_SIZE; i++)
		dbg_printf("%s %d %d %d\n", getStr(SECOND_PIPE(i).type), SECOND_PIPE(i).r1, SECOND_PIPE(i).r2, SECOND_PIPE(i).r3);
	dbg_printf("   ---- \n");
	
	

	
	
		
  } while(FIRST_PIPE(0).type!=undefined && SECOND_PIPE(0).type!=undefined);
  
  
  #else
  
  //Checar somente se existem instruções para fetch
  if(PIPELINE(0).type == undefined) {
	return;
  }
  // r1 só recebe dados
  // r2 e r3 só lê dados
  // Caso RAW
  if(PIPELINE(1).type == load && (PIPELINE(1).r1 == PIPELINE(0).r2 || PIPELINE(1).r1 == PIPELINE(0).r3)) {
    dbg_printf("DATA HAZARD DETECTED\n");
    bubble++;
  }
  
  dbg_printf("   ---- PIPELINE \n");
  for(int i = 0; i < PIPELINE_SIZE; i++)
    dbg_printf("%s %d %d %d\n", getStr(PIPELINE(i).type), PIPELINE(i).r1, PIPELINE(i).r2, PIPELINE(i).r3);
  dbg_printf("   ---- \n");
  #endif  
}

//taken-> 1 se fez o salto, 0 caso contrário
int taken;

//historico 
int hPredictor = 1;
//count eh usado no 2-bit predictor
int count;

//BR_PR 0-> sem branch predictor, 1-> alwaystaken, 2-> dynamic
#define BR_PR 1

void TestaControlHazard()
{
  if (PIPELINE(0).type == jump)
    {
      bubble += 2;
    }
  if(PIPELINE(0).type == branch) 
    {
      if (BR_PR == 1) //Alwways taken
	{
	  if(taken != hPredictor) 
	    {
	      bubble += 2;
	      return;
	    }
	}
      else if (BR_PR == 2)//2-bit predictor
	{
	  if (taken != hPredictor)
	    {
	      bubble += 2;
	      count++;
	      if (count >1)
		{
		  count = 0;
		  hPredictor = taken;
		}
	    }
	  else
	    count = 0;
	}
	else
		bubble += 2;
    }
  return;
}

void update()
{ 
  checkhazards();
  TestaControlHazard();
  
  #ifdef IS_SUPERESCALAR
  
  //Deslocar temporalmente o pipeline
  if( FIRST_PIPE(0).type != undefined && SECOND_PIPE(0).type != undefined){
	for (int i = PIPELINE_SIZE-1; i >= 1; i--){
		FIRST_PIPE(i) = FIRST_PIPE(i-1);
		SECOND_PIPE(i) = SECOND_PIPE(i-1); 
	}
	FIRST_PIPE(0) = SECOND_PIPE(0) = BUBBLE;
	current_pipe_index = 0;
  }  else if(FIRST_PIPE(0).type == undefined) {
	current_pipe_index = 0;
  } else {
	  current_pipe_index = 1;
  }
  
  #else
  //Deslocar temporalmente o pipeline
  for (int i = PIPELINE_SIZE-1; i >= 1; i--)
    PIPELINE(i) = PIPELINE(i-1);    
  PIPELINE(0) = null_instruction;
  
  #endif
}

/**#######################################################################**/
/**#######################################################################**/
/**#######################################################################**/

//!Generic instruction behavior method.
void ac_behavior( instruction )
{
  update();
  dbg_printf("----- PC=%#x ----- %lld\n", (int) ac_pc, ac_instr_counter);
  //  dbg_printf("----- PC=%#x NPC=%#x ----- %lld\n", (int) ac_pc, (int)npc, ac_instr_counter);
#ifndef NO_NEED_PC_UPDATE
  ac_pc = npc;
  npc = ac_pc + 4;
#endif 
};
 
//! Instruction Format behavior methods.
void ac_behavior( Type_R ){}
void ac_behavior( Type_I ){}
void ac_behavior( Type_J ){}
 
//!Behavior called before starting simulation
void ac_behavior(begin)
{
  #ifdef IS_SUPERESCALAR
  for(int i  = 0; i< PIPELINE_SIZE; i++){
		FIRST_PIPE(i).type = undefined;
		FIRST_PIPE(i).r1 = 0;
		FIRST_PIPE(i).r2 = 0;
		FIRST_PIPE(i).r3 = 0;
		
		SECOND_PIPE(i).type = undefined;
		SECOND_PIPE(i).r1 = 0;
		SECOND_PIPE(i).r2 = 0;
		SECOND_PIPE(i).r3 = 0;
  } 
  #else
  for(int i = 0; i < PIPELINE_SIZE; i++){
        PIPELINE(i).type = undefined;
		PIPELINE(i).r1 = 0;
		PIPELINE(i).r2 = 0;
		PIPELINE(i).r3 = 0;
  }
  #endif
  dbg_printf("@@@ begin behavior @@@\n");
  RB[0] = 0;
  npc = ac_pc + 4;

  // Is is not required by the architecture, but makes debug really easier
  for (int regNum = 0; regNum < 32; regNum ++)
    RB[regNum] = 0;
  hi = 0;
  lo = 0;

  RB[29] =  AC_RAM_END - 1024 - processors_started++ * DEFAULT_STACK_SIZE;
  hPredictor = 1;
}

//!Behavior called after finishing simulation
void ac_behavior(end)
{
  dbg_printf("@@@ end behavior @@@\n");
  dbg_printf(">>>> BUBBLES: %d\n", bubble);
  printf(">>>> BUBBLES: %d\n", bubble);
}

//!Instruction lb behavior method.
void ac_behavior( lb )
{
  setPipeline(load, rt, 0, 0);
  
  char byte;
  dbg_printf("lb r%d, %d(r%d)\n", rt, imm & 0xFFFF, rs);
  byte = DM.read_byte(RB[rs]+ imm);
  RB[rt] = (ac_Sword)byte ;
  dbg_printf("Result = %#x\n", RB[rt]);
};

//!Instruction lbu behavior method.
void ac_behavior( lbu )
{
  setPipeline(load, rt, 0, 0);
  
  unsigned char byte;
  dbg_printf("lbu r%d, %d(r%d)\n", rt, imm & 0xFFFF, rs);
  byte = DM.read_byte(RB[rs]+ imm);
  RB[rt] = byte ;
  dbg_printf("Result = %#x\n", RB[rt]);
};

//!Instruction lh behavior method.
void ac_behavior( lh )
{
  setPipeline(load, rt, 0, 0);
  
  short int half;
  dbg_printf("lh r%d, %d(r%d)\n", rt, imm & 0xFFFF, rs);
  half = DM.read_half(RB[rs]+ imm);
  RB[rt] = (ac_Sword)half ;
  dbg_printf("Result = %#x\n", RB[rt]);
};

//!Instruction lhu behavior method.
void ac_behavior( lhu )
{
  setPipeline(load, rt, 0, 0);
  
  unsigned short int  half;
  half = DM.read_half(RB[rs]+ imm);
  RB[rt] = half ;
  dbg_printf("Result = %#x\n", RB[rt]);
};

//!Instruction lw behavior method.
void ac_behavior( lw )
{
  setPipeline(load, rt, 0, 0);
  
  dbg_printf("lw r%d, %d(r%d)\n", rt, imm & 0xFFFF, rs);
  RB[rt] = DM.read(RB[rs]+ imm);
  dbg_printf("Result = %#x\n", RB[rt]);
};

//!Instruction lwl behavior method.
void ac_behavior( lwl )
{
  setPipeline(load, rt, 0, 0);
  
  dbg_printf("lwl r%d, %d(r%d)\n", rt, imm & 0xFFFF, rs);
  unsigned int addr, offset;
  ac_Uword data;

  addr = RB[rs] + imm;
  offset = (addr & 0x3) * 8;
  data = DM.read(addr & 0xFFFFFFFC);
  data <<= offset;
  data |= RB[rt] & ((1<<offset)-1);
  RB[rt] = data;
  dbg_printf("Result = %#x\n", RB[rt]);
};

//!Instruction lwr behavior method.
void ac_behavior( lwr )
{
  setPipeline(load, rt, 0, 0);
  
  dbg_printf("lwr r%d, %d(r%d)\n", rt, imm & 0xFFFF, rs);
  unsigned int addr, offset;
  ac_Uword data;

  addr = RB[rs] + imm;
  offset = (3 - (addr & 0x3)) * 8;
  data = DM.read(addr & 0xFFFFFFFC);
  data >>= offset;
  data |= RB[rt] & (0xFFFFFFFF << (32-offset));
  RB[rt] = data;
  dbg_printf("Result = %#x\n", RB[rt]);
};

//!Instruction sb behavior method.
void ac_behavior( sb )
{
  setPipeline(store, 0, rt, 0);
  
  unsigned char byte;
  dbg_printf("sb r%d, %d(r%d)\n", rt, imm & 0xFFFF, rs);
  byte = RB[rt] & 0xFF;
  DM.write_byte(RB[rs] + imm, byte);
  dbg_printf("Result = %#x\n", (int) byte);
};

//!Instruction sh behavior method.
void ac_behavior( sh )
{
  setPipeline(store, 0, rt, 0);
  
  unsigned short int half;
  dbg_printf("sh r%d, %d(r%d)\n", rt, imm & 0xFFFF, rs);
  half = RB[rt] & 0xFFFF;
  DM.write_half(RB[rs] + imm, half);
  dbg_printf("Result = %#x\n", (int) half);
};

//!Instruction sw behavior method.
void ac_behavior( sw )
{
  setPipeline(store, 0, rt, 0);
  
  dbg_printf("sw r%d, %d(r%d)\n", rt, imm & 0xFFFF, rs);
  DM.write(RB[rs] + imm, RB[rt]);
  dbg_printf("Result = %#x\n", RB[rt]);
};

//!Instruction swl behavior method.
void ac_behavior( swl )
{
  setPipeline(store, 0, rt, 0);
  
  dbg_printf("swl r%d, %d(r%d)\n", rt, imm & 0xFFFF, rs);
  unsigned int addr, offset;
  ac_Uword data;

  addr = RB[rs] + imm;
  offset = (addr & 0x3) * 8;
  data = RB[rt];
  data >>= offset;
  data |= DM.read(addr & 0xFFFFFFFC) & (0xFFFFFFFF << (32-offset));
  DM.write(addr & 0xFFFFFFFC, data);
  dbg_printf("Result = %#x\n", data);
};

//!Instruction swr behavior method.
void ac_behavior( swr )
{
  setPipeline(store, 0, rt, 0);
  
  dbg_printf("swr r%d, %d(r%d)\n", rt, imm & 0xFFFF, rs);
  unsigned int addr, offset;
  ac_Uword data;

  addr = RB[rs] + imm;
  offset = (3 - (addr & 0x3)) * 8;
  data = RB[rt];
  data <<= offset;
  data |= DM.read(addr & 0xFFFFFFFC) & ((1<<offset)-1);
  DM.write(addr & 0xFFFFFFFC, data);
  dbg_printf("Result = %#x\n", data);
};

//!Instruction addi behavior method.
void ac_behavior( addi )
{
  setPipeline(common, rt, rs, 0);
  
  dbg_printf("addi r%d, r%d, %d\n", rt, rs, imm & 0xFFFF);
  RB[rt] = RB[rs] + imm;
  dbg_printf("Result = %#x\n", RB[rt]);
  //Test overflow
  if ( ((RB[rs] & 0x80000000) == (imm & 0x80000000)) &&
       ((imm & 0x80000000) != (RB[rt] & 0x80000000)) ) {
    fprintf(stderr, "EXCEPTION(addi): integer overflow.\n"); exit(EXIT_FAILURE);
  }
};

//!Instruction addiu behavior method.
void ac_behavior( addiu )
{
  setPipeline(common, rt, rs, 0);
  
  dbg_printf("addiu r%d, r%d, %d\n", rt, rs, imm & 0xFFFF);
  RB[rt] = RB[rs] + imm;
  dbg_printf("Result = %#x\n", RB[rt]);
};

//!Instruction slti behavior method.
void ac_behavior( slti )
{
  setPipeline(common, rt, rs, 0);
  
  dbg_printf("slti r%d, r%d, %d\n", rt, rs, imm & 0xFFFF);
  // Set the RD if RS< IMM
  if( (ac_Sword) RB[rs] < (ac_Sword) imm )
    RB[rt] = 1;
  // Else reset RD
  else
    RB[rt] = 0;
  dbg_printf("Result = %#x\n", RB[rt]);
};

//!Instruction sltiu behavior method.
void ac_behavior( sltiu )
{
  setPipeline(common, rt, rs, 0);
  
  dbg_printf("sltiu r%d, r%d, %d\n", rt, rs, imm & 0xFFFF);
  // Set the RD if RS< IMM
  if( (ac_Uword) RB[rs] < (ac_Uword) imm )
    RB[rt] = 1;
  // Else reset RD
  else
    RB[rt] = 0;
  dbg_printf("Result = %#x\n", RB[rt]);
};

//!Instruction andi behavior method.
void ac_behavior( andi )
{
  setPipeline(common, rt, rs, 0);
  
  dbg_printf("andi r%d, r%d, %d\n", rt, rs, imm & 0xFFFF);
  RB[rt] = RB[rs] & (imm & 0xFFFF) ;
  dbg_printf("Result = %#x\n", RB[rt]);
};

//!Instruction ori behavior method.
void ac_behavior( ori )
{
  setPipeline(common, rt, rs, 0);
  
  dbg_printf("ori r%d, r%d, %d\n", rt, rs, imm & 0xFFFF);
  RB[rt] = RB[rs] | (imm & 0xFFFF) ;
  dbg_printf("Result = %#x\n", RB[rt]);
};

//!Instruction xori behavior method.
void ac_behavior( xori )
{
  setPipeline(common, rt, rs, 0);
  
  dbg_printf("xori r%d, r%d, %d\n", rt, rs, imm & 0xFFFF);
  RB[rt] = RB[rs] ^ (imm & 0xFFFF) ;
  dbg_printf("Result = %#x\n", RB[rt]);
};

//!Instruction lui behavior method.
void ac_behavior( lui )
{
  setPipeline(load, rt, 0, 0);
  
  dbg_printf("lui r%d, r%d, %d\n", rt, rs, imm & 0xFFFF);
  // Load a constant in the upper 16 bits of a register
  // To achieve the desired behaviour, the constant was shifted 16 bits left
  // and moved to the target register ( rt )
  RB[rt] = imm << 16;
  dbg_printf("Result = %#x\n", RB[rt]);
};

//!Instruction add behavior method.
void ac_behavior( add )
{
  setPipeline(common, rd, rs, rt);
  
  dbg_printf("add r%d, r%d, r%d\n", rd, rs, rt);
  RB[rd] = RB[rs] + RB[rt];
  dbg_printf("Result = %#x\n", RB[rd]);
  //Test overflow
  if ( ((RB[rs] & 0x80000000) == (RB[rd] & 0x80000000)) &&
       ((RB[rd] & 0x80000000) != (RB[rt] & 0x80000000)) ) {
    fprintf(stderr, "EXCEPTION(add): integer overflow.\n"); exit(EXIT_FAILURE);
  }
};

//!Instruction addu behavior method.
void ac_behavior( addu )
{
  setPipeline(common, rd, rs, rt);
  
  dbg_printf("addu r%d, r%d, r%d\n", rd, rs, rt);
  RB[rd] = RB[rs] + RB[rt];
  //cout << "  RS: " << (unsigned int)RB[rs] << " RT: " << (unsigned int)RB[rt] << endl;
  //cout << "  Result =  " <<  (unsigned int)RB[rd] <<endl;
  dbg_printf("Result = %#x\n", RB[rd]);
};

//!Instruction sub behavior method.
void ac_behavior( sub )
{
  setPipeline(common, rd, rs, rt);
  
  dbg_printf("sub r%d, r%d, r%d\n", rd, rs, rt);
  RB[rd] = RB[rs] - RB[rt];
  dbg_printf("Result = %#x\n", RB[rd]);
  //TODO: test integer overflow exception for sub
};

//!Instruction subu behavior method.
void ac_behavior( subu )
{
  setPipeline(common, rd, rs, rt);
  
  dbg_printf("subu r%d, r%d, r%d\n", rd, rs, rt);
  RB[rd] = RB[rs] - RB[rt];
  dbg_printf("Result = %#x\n", RB[rd]);
};

//!Instruction slt behavior method.
void ac_behavior( slt )
{
  setPipeline(common, rd, rs, rt);
  
  dbg_printf("slt r%d, r%d, r%d\n", rd, rs, rt);
  // Set the RD if RS< RT
  if( (ac_Sword) RB[rs] < (ac_Sword) RB[rt] )
    RB[rd] = 1;
  // Else reset RD
  else
    RB[rd] = 0;
  dbg_printf("Result = %#x\n", RB[rd]);
};

//!Instruction sltu behavior method.
void ac_behavior( sltu )
{
  setPipeline(common, rd, rs, rt);
  
  dbg_printf("sltu r%d, r%d, r%d\n", rd, rs, rt);
  // Set the RD if RS < RT
  if( RB[rs] < RB[rt] )
    RB[rd] = 1;
  // Else reset RD
  else
    RB[rd] = 0;
  dbg_printf("Result = %#x\n", RB[rd]);
};

//!Instruction instr_and behavior method.
void ac_behavior( instr_and )
{
  setPipeline(common, rd, rs, rt);
  
  dbg_printf("instr_and r%d, r%d, r%d\n", rd, rs, rt);
  RB[rd] = RB[rs] & RB[rt];
  dbg_printf("Result = %#x\n", RB[rd]);
};

//!Instruction instr_or behavior method.
void ac_behavior( instr_or )
{
  setPipeline(common, rd, rs, rt);
  
  dbg_printf("instr_or r%d, r%d, r%d\n", rd, rs, rt);
  RB[rd] = RB[rs] | RB[rt];
  dbg_printf("Result = %#x\n", RB[rd]);
};

//!Instruction instr_xor behavior method.
void ac_behavior( instr_xor )
{
  setPipeline(common, rd, rs, rt);
  
  dbg_printf("instr_xor r%d, r%d, r%d\n", rd, rs, rt);
  RB[rd] = RB[rs] ^ RB[rt];
  dbg_printf("Result = %#x\n", RB[rd]);
};

//!Instruction instr_nor behavior method.
void ac_behavior( instr_nor )
{
  setPipeline(common, rd, rs, rt);
  
  dbg_printf("nor r%d, r%d, r%d\n", rd, rs, rt);
  RB[rd] = ~(RB[rs] | RB[rt]);
  dbg_printf("Result = %#x\n", RB[rd]);
};

//!Instruction nop behavior method.
void ac_behavior( nop )
{  
  dbg_printf("nop\n");
};

//!Instruction sll behavior method.
void ac_behavior( sll )
{
  setPipeline(common, rd, rt, 0);
  
  dbg_printf("sll r%d, r%d, %d\n", rd, rs, shamt);
  RB[rd] = RB[rt] << shamt;
  dbg_printf("Result = %#x\n", RB[rd]);
};

//!Instruction srl behavior method.
void ac_behavior( srl )
{
  setPipeline(common, rd, rt, 0);
  
  dbg_printf("srl r%d, r%d, %d\n", rd, rs, shamt);
  RB[rd] = RB[rt] >> shamt;
  dbg_printf("Result = %#x\n", RB[rd]);
};

//!Instruction sra behavior method.
void ac_behavior( sra )
{
  setPipeline(common, rd, rt, 0);
  
  dbg_printf("sra r%d, r%d, %d\n", rd, rs, shamt);
  RB[rd] = (ac_Sword) RB[rt] >> shamt;
  dbg_printf("Result = %#x\n", RB[rd]);
};

//!Instruction sllv behavior method.
void ac_behavior( sllv )
{
  setPipeline(common, rd, rt, rs);
  
  dbg_printf("sllv r%d, r%d, r%d\n", rd, rt, rs);
  RB[rd] = RB[rt] << (RB[rs] & 0x1F);
  dbg_printf("Result = %#x\n", RB[rd]);
};

//!Instruction srlv behavior method.
void ac_behavior( srlv )
{
  setPipeline(common, rd, rt, rs);
  
  dbg_printf("srlv r%d, r%d, r%d\n", rd, rt, rs);
  RB[rd] = RB[rt] >> (RB[rs] & 0x1F);
  dbg_printf("Result = %#x\n", RB[rd]);
};

//!Instruction srav behavior method.
void ac_behavior( srav )
{
  setPipeline(common, rd, rt, rs);
  
  dbg_printf("srav r%d, r%d, r%d\n", rd, rt, rs);
  RB[rd] = (ac_Sword) RB[rt] >> (RB[rs] & 0x1F);
  dbg_printf("Result = %#x\n", RB[rd]);
};

//!Instruction mult behavior method.
void ac_behavior( mult )
{
  setPipeline(common, 0, rs, rt);
  
  dbg_printf("mult r%d, r%d\n", rs, rt);

  long long result;
  int half_result;

  result = (ac_Sword) RB[rs];
  result *= (ac_Sword) RB[rt];

  half_result = (result & 0xFFFFFFFF);
  // Register LO receives 32 less significant bits
  lo = half_result;

  half_result = ((result >> 32) & 0xFFFFFFFF);
  // Register HI receives 32 most significant bits
  hi = half_result ;

  dbg_printf("Result = %#llx\n", result);
};

//!Instruction multu behavior method.
void ac_behavior( multu )
{
  setPipeline(common, 0, rs, rt);
  
  dbg_printf("multu r%d, r%d\n", rs, rt);

  unsigned long long result;
  unsigned int half_result;

  result  = RB[rs];
  result *= RB[rt];

  half_result = (result & 0xFFFFFFFF);
  // Register LO receives 32 less significant bits
  lo = half_result;

  half_result = ((result>>32) & 0xFFFFFFFF);
  // Register HI receives 32 most significant bits
  hi = half_result ;

  dbg_printf("Result = %#llx\n", result);
};

//!Instruction div behavior method.
void ac_behavior( div )
{
  setPipeline(common, 0, rs, rt);
  
  dbg_printf("div r%d, r%d\n", rs, rt);
  // Register LO receives quotient
  lo = (ac_Sword) RB[rs] / (ac_Sword) RB[rt];
  // Register HI receives remainder
  hi = (ac_Sword) RB[rs] % (ac_Sword) RB[rt];
};

//!Instruction divu behavior method.
void ac_behavior( divu )
{
  setPipeline(common, 0, rs, rt);
  
  dbg_printf("divu r%d, r%d\n", rs, rt);
  // Register LO receives quotient
  lo = RB[rs] / RB[rt];
  // Register HI receives remainder
  hi = RB[rs] % RB[rt];
};

//!Instruction mfhi behavior method.
void ac_behavior( mfhi )
{
  setPipeline(common, rd, 0, 0);
  
  dbg_printf("mfhi r%d\n", rd);
  RB[rd] = hi;
  dbg_printf("Result = %#x\n", RB[rd]);
};

//!Instruction mthi behavior method.
void ac_behavior( mthi )
{
  setPipeline(common, 0, rs, 0);
  
  dbg_printf("mthi r%d\n", rs);
  hi = RB[rs];
  dbg_printf("Result = %#x\n", (unsigned int) hi);
};

//!Instruction mflo behavior method.
void ac_behavior( mflo )
{
  setPipeline(common, rd, 0, 0);
  
  dbg_printf("mflo r%d\n", rd);
  RB[rd] = lo;
  dbg_printf("Result = %#x\n", RB[rd]);
};

//!Instruction mtlo behavior method.
void ac_behavior( mtlo )
{
  setPipeline(common, 0, rs, 0);
  
  dbg_printf("mtlo r%d\n", rs);
  lo = RB[rs];
  dbg_printf("Result = %#x\n", (unsigned int) lo);
};

//!Instruction j behavior method.
void ac_behavior( j )
{
  setPipeline(jump, 0, 0, 0);
  
  dbg_printf("j %d\n", addr);
  addr = addr << 2;
#ifndef NO_NEED_PC_UPDATE
  npc =  (ac_pc & 0xF0000000) | addr;
#endif 
  dbg_printf("Target = %#x\n", (ac_pc & 0xF0000000) | addr );
};

//!Instruction jal behavior method.
void ac_behavior( jal )
{
  setPipeline(jump, 0, 0, 0);
  
  dbg_printf("jal %d\n", addr);
  // Save the value of PC + 8 (return address) in $ra ($31) and
  // jump to the address given by PC(31...28)||(addr<<2)
  // It must also flush the instructions that were loaded into the pipeline
  RB[Ra] = ac_pc+4; //ac_pc is pc+4, we need pc+8
	
  addr = addr << 2;
#ifndef NO_NEED_PC_UPDATE
  npc = (ac_pc & 0xF0000000) | addr;
#endif 
	
  dbg_printf("Target = %#x\n", (ac_pc & 0xF0000000) | addr );
  dbg_printf("Return = %#x\n", ac_pc+4);
};

//!Instruction jr behavior method.
void ac_behavior( jr )
{
  setPipeline(jump, 0, 0, 0);
  
  dbg_printf("jr r%d\n", rs);
  // Jump to the address stored on the register reg[RS]
  // It must also flush the instructions that were loaded into the pipeline
#ifndef NO_NEED_PC_UPDATE
  npc = RB[rs], 1;
#endif 
  dbg_printf("Target = %#x\n", RB[rs]);
};

//!Instruction jalr behavior method.
void ac_behavior( jalr )
{
  setPipeline(jump, 0, 0, 0);
  
  dbg_printf("jalr r%d, r%d\n", rd, rs);
  // Save the value of PC + 8(return address) in rd and
  // jump to the address given by [rs]

#ifndef NO_NEED_PC_UPDATE
  npc = RB[rs], 1;
#endif 
  dbg_printf("Target = %#x\n", RB[rs]);

  if( rd == 0 )  //If rd is not defined use default
    rd = Ra;
  RB[rd] = ac_pc+4;
  dbg_printf("Return = %#x\n", ac_pc+4);
};

//!Instruction beq behavior method.
void ac_behavior( beq )
{
  setPipeline(branch, 0, rt, rs);
  
  dbg_printf("beq r%d, r%d, %d\n", rt, rs, imm & 0xFFFF);
  if( RB[rs] == RB[rt] ){
    taken = 1;
#ifndef NO_NEED_PC_UPDATE
    npc = ac_pc + (imm<<2);
#endif 
    dbg_printf("Taken to %#x\n", ac_pc + (imm<<2));
  }
  else
    taken = 0;
};

//!Instruction bne behavior method.
void ac_behavior( bne )
{
  setPipeline(branch, 0, rt, rs);
  
  dbg_printf("bne r%d, r%d, %d\n", rt, rs, imm & 0xFFFF);
  if( RB[rs] != RB[rt] ){
    taken = 1;
#ifndef NO_NEED_PC_UPDATE
    npc = ac_pc + (imm<<2);
#endif 
    dbg_printf("Taken to %#x\n", ac_pc + (imm<<2));
  }	
  else
    taken = 0;
};

//!Instruction blez behavior method.
void ac_behavior( blez )
{
  setPipeline(branch, 0, rs, 0);
  
  dbg_printf("blez r%d, %d\n", rs, imm & 0xFFFF);
  if( (RB[rs] == 0 ) || (RB[rs]&0x80000000 ) ){
    taken = 1;
#ifndef NO_NEED_PC_UPDATE
    npc = ac_pc + (imm<<2), 1;
#endif 
    dbg_printf("Taken to %#x\n", ac_pc + (imm<<2));
  }	
  else
    taken = 0;
};

//!Instruction bgtz behavior method.
void ac_behavior( bgtz )
{
  setPipeline(branch, 0, rs, 0);
  
  dbg_printf("bgtz r%d, %d\n", rs, imm & 0xFFFF);
  if( !(RB[rs] & 0x80000000) && (RB[rs]!=0) ){
    taken = 1;
#ifndef NO_NEED_PC_UPDATE
    npc = ac_pc + (imm<<2);
#endif 
    dbg_printf("Taken to %#x\n", ac_pc + (imm<<2));
  }	
  else
    taken = 0;
};

//!Instruction bltz behavior method.
void ac_behavior( bltz )
{
  setPipeline(branch, 0, rs, 0);
  
  dbg_printf("bltz r%d, %d\n", rs, imm & 0xFFFF);
  if( RB[rs] & 0x80000000 ){
    taken = 1;
#ifndef NO_NEED_PC_UPDATE
    npc = ac_pc + (imm<<2);
#endif 
    dbg_printf("Taken to %#x\n", ac_pc + (imm<<2));
  }	
  else
    taken = 0;
};

//!Instruction bgez behavior method.
void ac_behavior( bgez )
{
  setPipeline(branch, 0, rs, 0);
  
  dbg_printf("bgez r%d, %d\n", rs, imm & 0xFFFF);
  if( !(RB[rs] & 0x80000000) ){
    taken = 1;
#ifndef NO_NEED_PC_UPDATE
    npc = ac_pc + (imm<<2);
#endif 
    dbg_printf("Taken to %#x\n", ac_pc + (imm<<2));
  }	
  else
    taken = 0;
};

//!Instruction bltzal behavior method.
void ac_behavior( bltzal )
{
  setPipeline(branch,  0, rs, 0);
  
  dbg_printf("bltzal r%d, %d\n", rs, imm & 0xFFFF);
  RB[Ra] = ac_pc+4; //ac_pc is pc+4, we need pc+8
  if( RB[rs] & 0x80000000 ){
    taken = 1;
#ifndef NO_NEED_PC_UPDATE
    npc = ac_pc + (imm<<2);
#endif 
    dbg_printf("Taken to %#x\n", ac_pc + (imm<<2));
  }	
  else
    taken = 0;
  dbg_printf("Return = %#x\n", ac_pc+4);
};

//!Instruction bgezal behavior method.
void ac_behavior( bgezal )
{
  setPipeline(branch,  0, rs, 0);
  
  dbg_printf("bgezal r%d, %d\n", rs, imm & 0xFFFF);
  RB[Ra] = ac_pc+4; //ac_pc is pc+4, we need pc+8
  if( !(RB[rs] & 0x80000000) ){
    taken = 1;
#ifndef NO_NEED_PC_UPDATE
    npc = ac_pc + (imm<<2);
#endif 
    dbg_printf("Taken to %#x\n", ac_pc + (imm<<2));
  }	
  else
    taken = 0;
  dbg_printf("Return = %#x\n", ac_pc+4);
};

//!Instruction sys_call behavior method.
void ac_behavior( sys_call )
{
  dbg_printf("syscall\n");
  stop();
}

//!Instruction instr_break behavior method.
void ac_behavior( instr_break )
{
  fprintf(stderr, "instr_break behavior not implemented.\n"); 
  exit(EXIT_FAILURE);
}
