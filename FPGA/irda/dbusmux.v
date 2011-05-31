`timescale 1ns / 1ps

module DBUSMUX ( DIUART0,  DIUART1, DIUART2, DIUART3, DIPIC, DICONS,
                 CSUART0, CSUART1, CSUART2, CSUART3, CSPIC, CSCONS,
                 nRW, DO);

parameter BW = 7;
//Inpus from peripherals
input [BW:0]  DIUART0;
input [BW:0]  DIUART1;
input [BW:0]  DIUART2;
input [BW:0]  DIUART3;
input [BW:0]  DIPIC;
input [BW:0]  DICONS;

//Chip select of peripherals
input CSUART0, CSUART1, CSUART2, CSUART3, CSPIC, CSCONS, nRW;

output [BW:0] DO;


reg    [BW:0] DOint;
wire cond_bus_out;

always @(CSUART0 or CSUART1 or CSUART2 or CSUART3 or CSPIC or
         DIUART0 or DIUART1 or DIUART2 or DIUART3 or DIPIC or  nRW)

begin
  case ({CSUART0, CSUART1, CSUART2, CSUART3, CSPIC})
    5'b10000: DOint=DIUART0;
    5'b01000: DOint=DIUART1;
    5'b00100: DOint=DIUART2;
    5'b00010: DOint=DIUART3;
    5'b00001: DOint=DIPIC;
    5'b00000: DOint=DICONS;
    default: DOint={(BW+1){1'bx}};
  endcase
end


assign
cond_bus_out=(~nRW)&(CSUART0|CSUART1|CSUART2|CSUART3|CSPIC),
DO=(cond_bus_out)? DOint:{(BW+1){1'bx}};


endmodule