////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 1995-2003 Xilinx, Inc.
// All Right Reserved.
////////////////////////////////////////////////////////////////////////////////
//   ____  ____ 
//  /   /\/   / 
// /___/  \  /    Vendor: Xilinx 
// \   \   \/     Version : 8.1.03i
//  \   \         Application : ISE
//  /   /         Filename : test.tfw
// /___/   /\     Timestamp : Sun Sep 24 11:15:19 2006
// \   \  /  \ 
//  \___\/\___\ 
//
//Command: 
//Design Name: test_tb_0
//Device: Xilinx
//
`timescale 1us/1ns

module test_tb_0;
    reg reset = 1'b0;
    reg CLK = 1'b0;
    reg [7:0] data_in = 8'b00000000;
    wire [7:0] data_out;
    reg [2:0] add = 3'b000;
    reg nRW = 1'b0;
    reg CS = 1'b0;
    reg RxD = 1'b0;
    wire TxD;
    wire nIRQ;
    reg CD = 1'b0;
    reg RI = 1'b0;
    reg DSR = 1'b0;
    reg CTS = 1'b0;
    wire DTR;
    wire RTS;

    parameter PERIOD = 200;
    parameter real DUTY_CYCLE = 0.5;
    parameter OFFSET = 0;

    initial    // Clock process for CLK
    begin
        #OFFSET;
        forever
        begin
            CLK = 1'b0;
            #(PERIOD-(PERIOD*DUTY_CYCLE)) CLK = 1'b1;
            #(PERIOD*DUTY_CYCLE);
        end
    end

    UART_PC UUT (
        .reset(reset),
        .CLK(CLK),
        .data_in(data_in),
        .data_out(data_out),
        .add(add),
        .nRW(nRW),
        .CS(CS),
        .RxD(RxD),
        .TxD(TxD),
        .nIRQ(nIRQ),
        .CD(CD),
        .RI(RI),
        .DSR(DSR),
        .CTS(CTS),
        .DTR(DTR),
        .RTS(RTS));

        integer TX_ERROR = 0;
        
        initial begin  // Open the results file...
            #1200 // Final time:  1200 us
            if (TX_ERROR == 0) begin
                $display("No errors or warnings.");
                end else begin
                    $display("%d errors found in simulation.", TX_ERROR);
                    end
                    $stop;
                end

                initial begin
                    // -------------  Current Time:  85us
                    #85;
                    reset = 1'b1;
                    // -------------------------------------
                    // -------------  Current Time:  485us
                    #400;
                    reset = 1'b0;
                    CD = 1'b1;
                    // -------------------------------------
                end

                task CHECK_data_out;
                    input [7:0] NEXT_data_out;

                    #0 begin
                        if (NEXT_data_out !== data_out) begin
                            $display("Error at time=%dns data_out=%b, expected=%b", $time, data_out, NEXT_data_out);
                            TX_ERROR = TX_ERROR + 1;
                        end
                    end
                endtask
                task CHECK_TxD;
                    input NEXT_TxD;

                    #0 begin
                        if (NEXT_TxD !== TxD) begin
                            $display("Error at time=%dns TxD=%b, expected=%b", $time, TxD, NEXT_TxD);
                            TX_ERROR = TX_ERROR + 1;
                        end
                    end
                endtask
                task CHECK_nIRQ;
                    input NEXT_nIRQ;

                    #0 begin
                        if (NEXT_nIRQ !== nIRQ) begin
                            $display("Error at time=%dns nIRQ=%b, expected=%b", $time, nIRQ, NEXT_nIRQ);
                            TX_ERROR = TX_ERROR + 1;
                        end
                    end
                endtask
                task CHECK_DTR;
                    input NEXT_DTR;

                    #0 begin
                        if (NEXT_DTR !== DTR) begin
                            $display("Error at time=%dns DTR=%b, expected=%b", $time, DTR, NEXT_DTR);
                            TX_ERROR = TX_ERROR + 1;
                        end
                    end
                endtask
                task CHECK_RTS;
                    input NEXT_RTS;

                    #0 begin
                        if (NEXT_RTS !== RTS) begin
                            $display("Error at time=%dns RTS=%b, expected=%b", $time, RTS, NEXT_RTS);
                            TX_ERROR = TX_ERROR + 1;
                        end
                    end
                endtask

            endmodule

