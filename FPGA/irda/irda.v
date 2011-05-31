	`timescale 1ns / 1ps
module irda(clk, D, A, nwe, ncs, nrd, reset, RxD0, RxD1, RxD2, RxD3,TxD0, TxD1, TxD2, TxD3, nIRQ, led);
parameter BW = 7;

input  clk, A, nwe, ncs, nrd, reset;
output nIRQ;
inout  [BW:0] D;		// To FPGA

input  RxD0, RxD1, RxD2, RxD3;
output TxD0, TxD1, TxD2, TxD3, led;

// Internal conection
 
wire   [9:0] A;
wire    led;


wire csuart0;
wire csuart1;
wire csuart2;
wire csuart3;
wire CSPIC;
wire nIRQ0;
wire nIRQ1;
wire nIRQ2;
wire nIRQ3;

wire [BW:0] duart0;
wire [BW:0] duart1;
wire [BW:0] duart2;
wire [BW:0] duart3;

wire [BW:0] DICONS;
wire [BW:0] DIPIC;
wire [BW:0] DTR;
wire [BW:0] RTS;
wire CD;
wire RI;
wire DSR;
wire CTS;

// *******************************************************************
//                       AT91 <-> FPGA INTERFACE
//        _______      ____________      _______
//       |       |    |            |    |       |
//       |     D |----|       rdBus|----|DOut   |
//       |     A |----|       weBus|----|DIn    |
//       |     WE|----| buffer_addr|----|A      |
//       |     RD|----|         nRW|----|RW     |
//       |     CS|----|            |    |       |
//       |       |    |            |    |       |
//       |_______|    |____________|    |_______|
//       AT91' BUS   AT91/FPGA Iface   Peripheric
//
   // synchronize signals                               
  reg   sncs, snwe;
  reg   [9:0] buffer_addr;
  reg   [BW:0] buffer_data;  

  reg   w_st;
  reg   nRW;

  reg   [BW:0] weBus;
  wire  [BW:0] rdBus;

  // Bi-directional bus controler
  wire         T = ~nwe | nrd | ncs;
  assign       D = T?{(BW+1){1'bZ}}:rdBus;

  // Sync Control, Data and adress buses
  always  @(negedge clk)
  begin
    sncs   <= ncs;
    snwe   <= nwe;

    buffer_data <= D;
	 buffer_addr <= A;
  end 

  // Write prrocess
  always @(posedge clk)
	if(reset) {w_st, nRW, weBus} <= 0;
	else begin
			weBus <= buffer_data;
			case (w_st)
				0: begin
					nRW <= 0;
					if(sncs | snwe) w_st <= 1;
				end
				1: begin
					if(~(sncs | snwe)) begin
						nRW    <= 1;
						w_st  <= 0;
					end	
					else nRW <= 0;
				end
			endcase
	end
	
	
	reg [32:0]  counter;
	always @(posedge clk) begin
	  if (reset)
	    counter <= {32{1'b0}};
	  else
	    counter <= counter + 1;
	end 
	assign led = counter[24];
	

//                     END AT91 <-> FPGA INTERFACE
// *******************************************************************



// Bus Multiplexer:  The XC3S don't has internal tri-states, if you have more than two
//                   Peripherics you must Multiplex its output Data Buses.
DBUSMUX busmux(duart0, duart1, duart2, duart3, DIPIC, DICONS, csuart0, csuart1, csuart2, csuart3,
                CSPIC, CSCONS, nRW, rdBus); // CS active high


// Peripheric instantiation
 UART_PC uart0 (reset, clk, weBus, duart0, buffer_addr[2:0], nRW, csuart0, RxD0, TxD0, nIRQ0, CD, RI, DSR, CTS, DTR[0], RTS[0]);
 UART_PC uart1 (reset, clk, weBus, duart1, buffer_addr[2:0], nRW, csuart1, RxD1, TxD1, nIRQ1, CD, RI, DSR, CTS, DTR[1], RTS[1]);
 UART_PC uart2 (reset, clk, weBus, duart2, buffer_addr[2:0], nRW, csuart2, RxD2, TxD2, nIRQ2, CD, RI, DSR, CTS, DTR[2], RTS[2]);
 UART_PC uart3 (reset, clk, weBus, duart3, buffer_addr[2:0], nRW, csuart3, RxD3, TxD3, nIRQ3, CD, RI, DSR, CTS, DTR[3], RTS[3]);

 DECODER deco1 (nwe, nrd, buffer_addr[5:3], ncs, csuart0, csuart1, csuart2, csuart3, CSPIC);
 PIC     pic1  ( weBus, DIPIC, buffer_addr[2:0] , {4'b1111, nIRQ0, nIRQ1, nIRQ2, nIRQ3}, nIRQ, CSPIC, nRW, clk, reset);
 
endmodule


  module DECODER (nwe, nrd, addr, ncs, csuart0, csuart1, csuart2, csuart3, CSPIC);
  input [2:0] addr;
  input ncs, nrd, nwe;
  
  output csuart0, csuart1, csuart2, csuart3, CSPIC;
  
  
  reg csuart0, csuart1, csuart2, csuart3, CSPIC;
  
  always @(addr, ncs, nwe, nrd)
  begin
    {csuart0, csuart1, csuart2, csuart3, CSPIC} <= 0;
	 if (~nwe | ~nrd) begin
     case ({ncs, addr})
	   4'b0000: {csuart0,csuart1,csuart2,csuart3, CSPIC} <= 5'b10000;
	   4'b0001: {csuart0,csuart1,csuart2,csuart3, CSPIC} <= 5'b01000;
	   4'b0010: {csuart0,csuart1,csuart2,csuart3, CSPIC} <= 5'b00100;
	   4'b0011: {csuart0,csuart1,csuart2,csuart3, CSPIC} <= 5'b00010;
		4'b0100: {csuart0,csuart1,csuart2,csuart3, CSPIC} <= 5'b00001;
	   default:{csuart0,csuart1,csuart2,csuart3, CSPIC} <= 5'b00000;
	  endcase
	 end
	 else 
	   {csuart0,csuart1,csuart2,csuart3, CSPIC} <= 5'b00000;
  end
  
  
  endmodule
