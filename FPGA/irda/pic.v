`timescale 1ns / 1ns
/******************************************************************************/
/* 				SIDSA					      */
/******************************************************************************/


module PIC   ( DI, DO, ADD, ISRC_LP, nIRQ, CS, nRW, MCLK, RESET); 

parameter BW = 7;

input  [BW:0] DI;
output [BW:0] DO;
input  [2:0]  ADD;
input  [BW:0] ISRC_LP;
output nIRQ;
input  CS, nRW;
input  MCLK, RESET;

//------------------------------
// registros internos


reg  nIRQ;
reg  [BW:0] DO;  //Registro de salida.
reg  [BW:0] IRQEnable;
wire [BW:0] ISRCF, IREG_LP;


assign  ISRCF   = ISRC_LP;
assign  IREG_LP = ( ~ISRCF  & IRQEnable); 



always @(posedge MCLK)
  begin
    nIRQ <= ~(|IREG_LP);
	end


always @(CS or ADD or nRW or IREG_LP or ISRCF or IRQEnable)
begin
      if (CS & ~nRW)
         begin
           case (ADD)
             3'b000: DO<=IREG_LP;          //IRQStatus
             3'b001: DO<=ISRCF;            //IRQRawStatus
             3'b010: DO<=IRQEnable;        //IRQEnable
             default:    DO<=32'b0;
           endcase
         end
      else DO<=32'b0;
end


always @(posedge MCLK or posedge RESET)
begin
 if (RESET)
    begin
      IRQEnable <= 32'b0;
    end
 else
    begin
      if (CS & nRW)
         begin
           case (ADD)
             3'b100: IRQEnable <= ( DI | IRQEnable); //EnableSet
             3'b101: IRQEnable <= (~DI & IRQEnable); //EnableClear      
             default: ;
           endcase
         end
    end
end




endmodule
