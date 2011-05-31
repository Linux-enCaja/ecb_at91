`timescale 1ns / 1ps


module sram_bus(clk, sram_data, addr, nwe, ncs, noe, reset);
  parameter     B            = (16-1);

  input            clk, addr, nwe, ncs, noe, reset;

  //FPGA
  inout [B:0]      sram_data;

  // synchronize signals                               
  reg          sncs, snwe;
  reg    [9:0] buffer_addr;
  reg    [B:0] buffer_data;  

  // interfaz fpga signals
  wire   [9:0] addr;
	
  // bram interfaz signals
  reg          we;
  reg          w_st;

  reg    [B:0] wdBus;
  wire   [B:0] rdBus;

  // interefaz signals assignments
  wire         T = ~nwe | noe | ncs;
  assign       sram_data = T?16'bZ:rdBus;
  //assign       sram_data = T?16'bZ:16'hcabf;
  
  //--------------------------------------------------------------------------

  // synchronize assignment
  always  @(negedge clk)
  begin
    sncs   <= ncs;
    snwe   <= nwe;

    buffer_data <= sram_data;
	 buffer_addr <= addr;
  end


	// write access pxa to bram
  always @(posedge clk)
	if(reset) {w_st, we, wdBus} <= 0;
	else begin
			wdBus <= buffer_data;
			case (w_st)
				0: begin
					we <= 0;
					if(sncs | snwe) w_st <= 1;
				end
				1: begin
					if(~(sncs | snwe)) begin
						we    <= 1;
						w_st  <= 0;
					end	
					else we <= 0;
				end
			endcase
	end	

RAMB16_S18 ba0( .CLK(~clk), .EN(1'b1),   .SSR(1'b0),      .ADDR(buffer_addr), 
                .WE(we),    .DIP(2'b00), .DI(wdBus), .DO(rdBus) );  
endmodule

