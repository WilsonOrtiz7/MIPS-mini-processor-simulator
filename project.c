//Wilson Ortiz
//Christian Samson
//Andrew Bryants
//Group 58
//CDA3103 Final Project
#include <stdio.h>
#include "spimcore.h"
enum operation{ADD = '0', SUB = '1', LESS_THAN = '2', UNSIGNED_LESS_THAN = '3', AND = '4', OR = '5', SHIFT_LEFT_16_BITS = '6', NOT = '7'} operation;

void ALU(unsigned A,unsigned B,char ALUControl,unsigned *ALUresult,char *Zero){
    //cases refer to chars '0'-'7' created in enum operation
    //funct should 0 for add, 1 for subtraction and so on in order for this to work
    switch (ALUControl){
        case ADD:
            *ALUresult = A + B;
            break;

        case SUB:
            *ALUresult = A - B;
            break;
            
        case LESS_THAN:
            *ALUresult = ((signed)A < (signed) B);
            break;
            
        case UNSIGNED_LESS_THAN:
            *ALUresult = (A < B);
            break;

        case AND:
            *ALUresult = A & B;
            break;

        case OR:
            *ALUresult = A | B;
            break;

        case SHIFT_LEFT_16_BITS:
            *ALUresult = B << 16;
            break;

        case NOT:
            *ALUresult = !A;
            break;
    }
    //checks if the ALUresult is 0 and sets the Zero flag accordingly
    if(*ALUresult == 0){
        *Zero = '1';
    }
    else{
       *Zero = '0';
    }

}

int instruction_fetch(unsigned PC,unsigned *Mem,unsigned *instruction)
{
  //check proper section of the memory index
  unsigned data = Mem[PC >> 2];
  *instruction = data;
  // not a multiple of 4, or outside of code segment
  if(PC % 4 != 0 || PC < 0x4000 || PC > 0xffff){
    return 1;
  }
  else
  {
    return 0;
  }
}

void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
{
  //apply shifts to get the values we want
  *op = (instruction>>26) & 0x0000003F;
  *r1 = (instruction>>21) & 0x0000001F;
  *r2 = (instruction>>16) & 0x0000001F;
  *r3 = (instruction>>11) & 0x0000001F;
  *funct = instruction & 0x0000003F;
  *offset = instruction & 0x0000FFFF;
  *jsec = instruction & 0x03FFFFFF;
}


int instruction_decode(unsigned op, struct_controls *controls)
{
  //add, and, sub, or, sltu 
  if(op == 0){
    controls->RegDst = '1';
    controls->RegWrite = '1';
    controls->ALUSrc = '0';
    controls->ALUOp = '7';   //r-type
    controls->MemWrite = '0';
    controls->MemRead = '0';
    controls->MemtoReg = '0';
    controls->Jump = '0';
    controls->Branch = '0';

    return 0;
  }

  //j
  if(op == 2){
    controls->RegDst = '0';
    controls->RegWrite = '0';
    controls->ALUSrc = '0';
    controls->ALUOp = '0'; // dont care
    controls->MemWrite = '0';
    controls->MemRead = '0';
    controls->MemtoReg = '0';
    controls->Jump ='1';
    controls->Branch = '0';

    return 0;
  }

  //beq
  if(op == 4){
    controls->RegDst = '2';
    controls->RegWrite = '0';
    controls->ALUSrc = '0';
    controls->ALUOp = '1';  //alu will do subtraction
    controls->MemWrite = '0';
    controls->MemRead = '0';
    controls->MemtoReg = '2';
    controls->Jump = '0';
    controls->Branch ='1';
    return 0;
  }

  //addi
  if (op == 8){
    controls->RegDst = '0';
    controls->RegWrite = '1';
    controls->ALUSrc = '1';
    controls->ALUOp = '0';  // do addition or dont care
    controls->MemWrite = '0';
    controls->MemRead = '0';
    controls->MemtoReg = '0';
    controls->Jump = '0';
    controls->Branch ='0';
    return 0;
  }

  //slti
  if(op == 10){
    controls->RegDst = '0';
    controls->RegWrite = '1';
    controls->ALUSrc = '1';
    controls->ALUOp = '2'; //alu will do set less than 
    controls->MemWrite = '0';
    controls->MemRead = '0';
    controls->MemtoReg = '0';
    controls->Jump = '0';
    controls->Branch = '0';

    return 0;
  }

  //sltiu
  if(op == 11){
    controls->RegDst = '0';
    controls->RegWrite = '1';
    controls->ALUSrc = '1';
    controls->ALUOp = '3'; // alu will do set less than unsigned operation
    controls->MemWrite = '0';
    controls->MemRead = '0';
    controls->MemtoReg = '0';
    controls->Jump = '0';
    controls->Branch = '0';

    return 0;
  }

  //lw
  if(op == 35){
    controls->RegDst = '0';
    controls->RegWrite = '1';
    controls->ALUSrc = '1';
    controls->ALUOp = '0';  //alu will do addition
    controls->MemWrite = '0';
    controls->MemRead = '1';
    controls->MemtoReg = '1';
    controls->Jump = '0';
    controls->Branch = '0';

    return 0;
  }

  //lui
  if(op == 15){
    controls->RegDst = '0';
    controls->RegWrite = '1';
    controls->ALUSrc = '1';
    controls->ALUOp = '6'; //shift left extend vaulue by 16 bits
    controls->MemWrite = '0';
    controls->MemRead = '0';
    controls->MemtoReg = '0';
    controls->Jump = '0';
    controls->Branch ='0';

    return 0;
  }

  //sw
  if(op == 43){
    controls->RegDst = '2';
    controls->RegWrite = '0';
    controls->ALUSrc = '1';
    controls->ALUOp = '0'; //alu will do addition 
    controls->MemWrite = '1';
    controls->MemRead = '0';
    controls->MemtoReg = '2';
    controls->Jump ='0';
    controls->Branch ='0';

    return 0;
  }
  return 1;
}

void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2)
{
  // take the value inside regisiters and reads to memory 
  *data1 = Reg[r1];
  *data2 = Reg[r2];
}

void sign_extend(unsigned offset,unsigned *extended_value){
   unsigned extend = 0xFFFF0000;
   unsigned negative = offset >> 15;
  //if it is negative we then sign extend with 1s
   if(negative == 1)
      *extended_value = offset | extend;
  //if number is not sign extend sign extend normally 
  else
      *extended_value = offset & 0x0000ffff;
}

int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,unsigned funct,char ALUOp,char ALUSrc,unsigned *ALUresult,char *Zero){
    
    // r-type
    if(ALUOp == '7'){

      switch(funct){
        case 32:
          ALU(data1, data2, ADD, ALUresult, Zero);
          break;

        case 34:
          ALU(data1, data2, SUB, ALUresult, Zero);
          break;

        case 42:
          ALU(data1, data2, LESS_THAN, ALUresult, Zero);
          break;

        case 43:
          ALU(data1, data2, UNSIGNED_LESS_THAN, ALUresult, Zero);
          break;
          
        case 36:
          ALU(data1, data2, AND, ALUresult, Zero);
          break;

        case 37:
          ALU(data1, data2, OR, ALUresult, Zero);
          break;

        default:
          return 1;
      }
      return 0;
    }

    switch(ALUOp){
      // lw
      case '0':
        ALU(data1, extended_value, ADD, ALUresult, Zero); 
        break;
      // beq
      case '1':
        ALU(data1, data2, SUB, ALUresult, Zero);
        break;
      //stli
      case '2':
        ALU(data1, extended_value, LESS_THAN, ALUresult, Zero);
        break;
        // sltiu
      case '3':
        ALU(data1, extended_value, UNSIGNED_LESS_THAN, ALUresult, Zero);
        break;

      case '6':
        ALU(data1, extended_value, SHIFT_LEFT_16_BITS, ALUresult, Zero);
        break;
      // not
      case '7':
        ALU(data1, data2, NOT, ALUresult, Zero);
        break;

      default: 
        return 1;
    }
    return 0;
}  

int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned*memdata,unsigned *Mem)
{
  //writes to data 2 to memory
  //also cheek for word alinigned 
  if (MemWrite == '1')
  {
    if(ALUresult % 4 != 0){
      return 1;
    }
    Mem[ALUresult>>2] = data2;
    // writes what number is in data 2 to the given addrese
    return 0;
  }
  // case MemRead = 1 
  if (MemRead == '1')
  {
    if(ALUresult % 4 != 0){
      return 1;
    }
    *memdata = Mem[ALUresult>>2];
    //memdata = what every was in the addrese given 
    return 0;
  }
return  0;
}

void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{
  //checks if access is given to write to regisiter 
  // based on MemtoReg and RegDst
  // decides what what reg to write to 
  // and what to write 
  if (RegWrite == '1')
      {
         if (MemtoReg == '0' && RegDst == '0')
         {
           Reg[r2] = ALUresult;
         }
         if (MemtoReg == '1' && RegDst == '1') 
         {
            Reg[r3] = memdata;
         }
         if(MemtoReg == '1' && RegDst == '0')
         {
           Reg[r2] = memdata;
         }
         if(MemtoReg == '0' && RegDst == '1')
         {
           Reg[r3] = ALUresult;
         }
      }

}

void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{
  //always PC_update
  *PC = *PC + 4;
       //branch taken  
    if (Branch == '1' && Zero == '1')
        {
          *PC = *PC + (4 * extended_value);
        }
        // if jump is taken 
        if (Jump == '1')
        {
          jsec = jsec << 2;
          *PC = jsec | (*PC & 0xf0000000);

        }    
  
}