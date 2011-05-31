`timescale 1ns / 1ps

module sram_TB_v;

	// Inputs
	reg clk;
	reg [9:0] addr;
	reg nwe;
	reg ncs;
	reg noe;
	reg reset;

	// Bidirs
    reg [15:0] sram_data$inout$reg ;
    wire [15:0] sram_data = sram_data$inout$reg;

	// Instantiate the Unit Under Test (UUT)
	sram_bus uut (.clk(clk), .sram_data(sram_data), .addr(addr), .nwe(nwe), .ncs(ncs), .noe(noe),
						.reset(reset));

	 parameter PERIOD  = 20;
    parameter real DUTY_CYCLE = 0.5;
    parameter OFFSET  = 0;
	 parameter TSET    = 3;
	 parameter THLD    = 3;
	 parameter NWS     = 3;
	 
	 reg [15:0] i;
	 reg [15:0] data_tx;	 


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
	 
	initial begin
		// Initialize Inputs
		clk = 0; addr = 0; nwe = 1; ncs = 1; noe = 1;
    end

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

    
	 
    initial begin: TEST_CASE	
	 
	  #10 -> reset_trigger;
	  @ (reset_done_trigger); 
	  
	  // Write data to SRAM
	   for(i=0; i<10; i=i+1)
		begin
		    @ (posedge clk);
			 ncs = 0;
			 addr <= i[9:0];
			 repeat (TSET) begin
				@ (posedge clk);
			 end	 
			 nwe = 0;
			 sram_data$inout$reg 	<= i;
			 repeat (NWS) begin
				@ (posedge clk);
			 end
			 nwe = 1;
			 repeat (THLD) begin
				@ (posedge clk);
			 end
			 ncs = 1;
			 sram_data$inout$reg = {16{1'bz}};
		end

		nwe = 1;
		
		//Read Data

	   for(i=0; i<10; i=i+1)
		begin
		    @ (posedge clk);
			 ncs = 0;
			 addr <= i[9:0];
			 repeat (TSET) begin
				@ (posedge clk);
			 end	 
			 noe = 0;
			 sram_data$inout$reg 	<= i;
			 repeat (NWS) begin
				@ (posedge clk);
			 end
			 noe = 1;
			 repeat (THLD) begin
				@ (posedge clk);
			 end
			 ncs = 1;
			 sram_data$inout$reg = {16{1'bz}};
		end


	 end
	
	
      
endmodule

