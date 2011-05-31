`timescale 1ns / 1ps

module irda_TB_v;

	// Inputs
	reg clk; 
	reg [9:0] A;
	reg nwe;
	reg ncs;
	reg nrd;
	reg reset;
	reg RxD0;
	reg RxD1;
	reg RxD2;
	reg RxD3;

	// Outputs
	wire TxD0;
	wire TxD1;
	wire TxD2;
	wire TxD3;
	wire nIRQ;
	wire led;

	// Bidirs
    reg  [7:0] D$inout$reg ;
    wire [7:0] D = D$inout$reg;	

	// Instantiate the Unit Under Test (UUT)
	irda uut (.clk(clk), .D(D), .A(A), .nwe(nwe), .ncs(ncs), .nrd(nrd), .reset(reset), .RxD0(RxD0), 
		.RxD1(RxD1), .RxD2(RxD2), .RxD3(RxD3), .TxD0(TxD0), .TxD1(TxD1), .TxD2(TxD2), .TxD3(TxD3), .nIRQ(nIRQ),
		.led(led)
	);

	initial begin
		// Initialize Inputs
		clk = 0; A = 0; nwe = 1; ncs = 1; nrd = 1; reset = 1; RxD0 = 1; RxD1 = 1; RxD2 = 1; RxD3 = 1;
	end


	 parameter TBIT  = 432;		//Bit time
	 parameter PERIOD = 20;
    parameter real DUTY_CYCLE = 0.5;
    parameter OFFSET = 0;
	 parameter TSET    = 3;
	 parameter THLD    = 3;
	 parameter NWS     = 3;	 
	 
	 
	 reg [15:0] i;
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
	  @ (reset_done_trigger); 

    //Configure PIC: Enable uarts interrupts
		 @ (posedge clk);
		 ncs = 0;
		 A <= 6'b100100;         // Write IRQEnable
		 repeat (TSET) begin
			@ (posedge clk);
		 end	 
		 nwe = 0;
		 D$inout$reg 	<= (8'h08);
		 repeat (NWS) begin
			@ (posedge clk);
		 end
		 nwe = 1;
		 repeat (THLD) begin
			@ (posedge clk);
		 end
		 ncs = 1;
		 D$inout$reg = {8{1'bz}};	 
	 
	 

       // Enable Reception Interrupt
		 @ (posedge clk);
		 ncs = 0;
		 A <= 3'b001;
		 repeat (TSET) begin
			@ (posedge clk);
		 end	 
		 nwe = 0;
		 D$inout$reg 	<= (8'h04);
		 repeat (NWS) begin
			@ (posedge clk);
		 end
		 nwe = 1;
		 repeat (THLD) begin
			@ (posedge clk);
		 end
		 ncs = 1;
		 D$inout$reg = {8{1'bz}};		 
		 

       // Send Data for all uarts
       for(i=0; i<4; i=i+1)
       begin	  
     
		    @ (posedge clk);
			 ncs = 0;
			 A <= (i*8);
			 repeat (TSET) begin
				@ (posedge clk);
			 end	 
			 nwe = 0;
			 D$inout$reg 	<= (8'hAA)*i;
			 repeat (NWS) begin
				@ (posedge clk);
			 end
			 nwe = 1;
			 repeat (THLD) begin
				@ (posedge clk);
			 end
			 ncs = 1;
			 D$inout$reg = {8{1'bz}};
	   end
		
		//Incoming data Test
		data_tx <= 10'b1100101100;
		for(i=0; i<10; i=i+1)
		begin
	     repeat(TBIT) begin
	       @(negedge clk);
	     end		
		  RxD0 <= data_tx[i];
		  RxD1 <= data_tx[i];
		  RxD2 <= data_tx[i];
		  RxD3 <= data_tx[i];
	   end		
		RxD0 <= 1; RxD1 <= 1; RxD2 <= 1; RxD3 <= 1;

		@(negedge nIRQ);    // wait for nIRQ
		
		for(i=0; i<4; i=i+1)
		begin      
		  @ (posedge clk);
		  ncs = 0;
		  A <= (i*8);
		  repeat (TSET) begin
			@ (posedge clk);
		  end	 
		  nrd = 0;
		  A <= (i*8);
		  repeat (NWS) begin
			@ (posedge clk);
		  end
		  nrd = 1;
		  repeat (THLD) begin
			 @ (posedge clk);
		  end
		  ncs = 1;
		  D$inout$reg = {8{1'bz}};		
		end

		//Incoming data Test 2
		data_tx <= 10'b1110011000;
		for(i=0; i<10; i=i+1)
		begin
	     repeat(TBIT) begin
	       @(negedge clk);
	     end		
		  RxD0 <= data_tx[i];
		  RxD1 <= data_tx[i];
		  RxD2 <= data_tx[i];
		  RxD3 <= data_tx[i];
	   end		
		RxD0 <= 1; RxD1 <= 1; RxD2 <= 1; RxD3 <= 1;

      // wait for nIRQ
		@(negedge nIRQ);       // read Data for de-assert nIRQ
		for(i=0; i<4; i=i+1)
		begin      
		  @ (posedge clk);
		  ncs = 0;
		  A <= (i*8);
		  repeat (TSET) begin
			@ (posedge clk);
		  end	 
		  nrd = 0;
		  A <= (i*8);
		  repeat (NWS) begin
			@ (posedge clk);
		  end
		  nrd = 1;
		  repeat (THLD) begin
			 @ (posedge clk);
		  end
		  ncs = 1;
		  D$inout$reg = {8{1'bz}};		
		end
		
		       // Send Data 2 for all uarts
       for(i=0; i<4; i=i+1)
       begin	  
     
		    @ (posedge clk);
			 ncs = 0;
			 A <= (i*8);
			 repeat (TSET) begin
				@ (posedge clk);
			 end	 
			 nwe = 0;
			 D$inout$reg 	<= (8'h11)*i;
			 repeat (NWS) begin
				@ (posedge clk);
			 end
			 nwe = 1;
			 repeat (THLD) begin
				@ (posedge clk);
			 end
			 ncs = 1;
			 D$inout$reg = {8{1'bz}};
	   end

	 
	 end // TESTCASE
	  

      
endmodule

