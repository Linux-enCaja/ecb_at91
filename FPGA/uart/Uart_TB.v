`timescale 1ns / 1ps

module Uart_TB_v;

	// Inputs
	reg reset;
	reg clk;
	reg [7:0] data_in;
	reg [2:0] add;
	reg nRW;
	reg CS;
	reg RxD;
	reg CD;
	reg RI;
	reg DSR;
	reg CTS;

	// Outputs
	wire [7:0] data_out;
	wire TxD;
	wire nIRQ;
	wire DTR;
	wire RTS;
	
	
	//
	

	// Instantiate the Unit Under Test (UUT)
	UART_PC uut (
		.reset(reset), .CLK(clk),  .data_in(data_in), .data_out(data_out), 
		.add(add), .nRW(nRW), .CS(CS), .RxD(RxD), .TxD(TxD), .nIRQ(nIRQ), 
		.CD(CD), .RI(RI), .DSR(DSR), .CTS(CTS), .DTR(DTR), .RTS(RTS) );

	initial begin
		// Initialize Inputs
		reset = 0; clk = 0; data_in = 0; add = 0; nRW = 0; CS = 0; RxD = 1;
		CD = 0; RI = 0; DSR = 0; CTS = 0;
	end

    
	 parameter TBIT  = 432;		//Bit time
	 parameter PERIOD = 20;
    parameter real DUTY_CYCLE = 0.5;
    parameter OFFSET = 0;
	 
	 reg [3:0] i;
	 reg [9:0] data_tx;

    initial    // Clock process for clk
    begin
        #OFFSET;
        forever
        begin
            clk = 1'b0;
            #(PERIOD-(PERIOD*DUTY_CYCLE)) clk = 1'b1;
            #(PERIOD*DUTY_CYCLE);
        end
    end

	event reset_trigger;
	event reset_done_trigger;

	initial begin 
	  forever begin 
	   @ (reset_trigger);
		@ (negedge clk);
		reset = 1;
		@ (negedge clk);
		reset = 0;
		-> reset_done_trigger;
		end
	end
	
	initial begin: TEST_CASE 
	  #10 -> reset_trigger;
	  // Init TX transmition
	  @ (reset_done_trigger); CS = 1; data_in = 8'hAA;
	  @ (negedge clk); add = 6'b000; nRW = 1;
	  @ (negedge clk); add = 6'b000; nRW = 0; CS = 0;
	  
	  //Simulate Serial reception
		data_tx <= 10'b1100101100;
		for(i=0; i<10; i=i+1)
		begin
	     repeat(TBIT) begin
	       @(negedge clk);
	     end		
		  RxD <= data_tx[i];
	   end		
		RxD <= 1;

   end	
      
endmodule

//{8{1}}


