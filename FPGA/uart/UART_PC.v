/******************************************************************************/
/* 				SIDSA					      */
/******************************************************************************/
/* HSDT100 ARM PERIPHERALS						      */
/*									      */
/* MODULE: UART				 				      */
/*									      */
/* FILE: UART_PC.v		VERSION:3.0	DATE: 19-I-98	      	      */
/******************************************************************************/

 
module UART_PC (reset, CLK, data_in, data_out, add, nRW, CS,
		RxD, TxD, nIRQ, CD, RI, DSR, CTS, DTR, RTS);

input		reset;		// Reset (H)
input		CLK;		// System clock 
input   [7:0]   data_in;        // Input Data Bus
output  [7:0]   data_out;       // Output Data Bus
input   [2:0]   add;            // Address Bus
input           nRW;            // Read_ / Write from ARM
input           CS;             // UART Chip Select
output		nIRQ;		// Interrupt Request Output to ARM

input		RxD;		// Receiver Data Line 
output          TxD;            // Transmiter Data Line
input		CD;		// Carrier Detect
input		RI;		// Ring Indicator
input		DSR;		// Data Send Ready
input		CTS;		// Clear To Send
output		RTS;		// Request To Send
output		DTR;		// Data Terminal Ready


// Variables
//.............................................................

wire	[7:0]   dato_rx;        // Dato recibido
wire	[7:0]   dato_tx;        // Dato para transmitir
wire		err_paridad;    // Error de paridad (H)
wire		err_frame;      // Error de trama (H)
wire		err_overrun;    // Error de rebosamiento (H)
wire		error;          // Error (OR de los anteriores)
wire		rx_lleno;       // Dato disponible
wire		tx_empty;       // Transmisor vacio
wire		txt_empty;      // Transmisor completamente vacio
wire		borrar_rdy;	// Borrar dato_rdy tras leer DATO_RX
wire		borrar_err;	// Borrar err_over tras leer STATUS_RX
wire		carga;          // Senal carga
wire		carga_div;      // Senal carga cte_div

wire		clkl;		// Frecuencia de reloj dividida
wire		clkls;		// Reloj de muestreo sincronizado (el nivel
				// alto dura un periodo de reloj)
wire		RxDs;		// Senal RxD sincronizada y limpia
wire		sample;		// Impulso de muestreo de la senal RxDs
wire		samples;	// Impulso de muestreo de RxDs conformado
wire		load_shift;

wire		carga_IER;	// senal carga IER
wire		carga_MCR;	// senal carga MCR
wire		carga_LCR;	// senal carga LCR
wire		carga_MSR;	// senal carga MSR (para codificacion delta)
wire		carga_ISR;	// senal carga ISR

wire	[1:0]	WordLength;	// CONFIGURACIONES UART
wire		Stop;
wire		ParityEnable;
wire		Parity;
wire		ParityForced;
wire		Break;
wire		BaudSelect;	
wire		modem_int;
wire	[7:0]	modem;
wire	[3:0]	ISR;
wire	[7:0]	LSR;
wire	[7:0]	LCR;
wire	[3:0]	IER;
//.............................................................


//  The default comunication speed is 115200 (clk = 50MHz); divide the clock if you want another spped
//  The Minimun Clk frequency is 50Mz
//  50000000/115200 = 434 => 16 X 27 = 432

pc_if_arm_pc if_arm1 (reset, CLK, data_in, data_out, add, nRW, CS,
                borrar_rdy, borrar_err, 
		carga, carga_div_low, carga_div_high,
		carga_IER, carga_MCR, 
		carga_LCR, carga_MSR, carga_ISR,
                BaudSelect, dato_rx, modem, ISR, LSR, LCR);

pc_div27 div27(reset, CLK, 1'b1, clk_pres);

pc_div_ms div_ms1 (reset, CLK, clk_pres,data_in, carga_div_low, carga_div_high, clkl);

pc_pulso pulso1(reset, CLK, clkl, clkls);

pc_div16 div161(reset, CLK, clkls, clktx);

pc_ifrxd ifrxd1(reset, CLK, clkls, RxD, RxDs);

pc_muestreo muestreo1(reset, CLK, rx_lleno, clkls, RxDs, sample);

pc_pulso pulso2(reset, CLK, sample, samples);

pc_buffrx_pc buffrx1(reset, CLK, RxDs, samples, rx_lleno, dato_rx, err_paridad,
		err_frame, ParityEnable, Parity, ParityForced);

pc_ctrl_rx ctrl_rx1(reset, CLK, samples, rx_lleno);

pc_dato_rdy dato_rdy1(reset, CLK, rx_lleno, borrar_rdy, borrar_err, dato_rdy,
		err_overrun);

pc_pulso pulso3(reset, CLK, clktx, clktxs);

pc_bufftx bufftx1 (reset, CLK, carga, load_shift, clktxs, data_in, TxD,
		ParityEnable, Parity, ParityForced);

pc_ctrl_tx_pc ctrl_tx1 (reset, CLK, carga, clktxs, load_shift, tx_empty,
		txt_empty);

//pc_modem m1(reset, CLK, carga_MCR, carga_MSR, data_in, modem,
//         CD, RI, DSR, CTS, DTR, RTS, modem_int); 

pc_ier   ier1(reset, CLK, carga_IER, data_in, dato_rdy, tx_empty, modem_int, nIRQ, IER);

pc_lcr   lcr1(reset, CLK, LCR, carga_LCR, data_in, 
             WordLength, Stop, ParityEnable, Parity,
	     ParityForced, Break, BaudSelect); 

pc_isr   isr1(reset, CLK, carga_ISR, 
	     err_paridad, err_frame, err_overrun, 
	     dato_rdy, tx_empty, txt_empty, modem_int, 
             ISR, LSR, IER); 

endmodule







//............................................................
// BUFFER RECEIVER
//
//............................................................
 
 
module pc_buffrx_pc (reset, CLK, RxDs, samples, fin_rx, datorx, 
		err_paridad, err_frame,
		ParityEnable, Parity, ParityForced);

input		reset;		// Reset (H)
input		CLK;		// Reloj del sistema
input		RxDs;		// Linea RxD limpia 
input		samples;	// Impulso de muestreo de RxDs conformado 
input		fin_rx;		// Indicador (L) de recepcion en curso
output	[7:0]	datorx;		// Dato recibido
output		err_paridad; 	// Error de paridad (H)
output		err_frame; 	// Error en el bit de parada (H)
input		ParityEnable;	// Habilitacion de paridad 
input		Parity;		// Paridad
input		ParityForced;	// Valor de paridad forzada.


// Variables
//.............................................................

reg	[9:0]	bufrx;		// Registro serie-paralelo del Receptor
				// bufrx[7:0]=dato, bufrx[8]=paridad
				// bufrx[9]=parada
reg	[7:0]	datorx;		// Dato recibido
reg		err_paridad; 	// Error de paridad (H)
reg		err_frame; 	// Error en el bit de parada (H)
wire		iparity;

//.............................................................

// Si el numero de 1s es par, iparity =1

assign iparity =  bufrx[8] ^ bufrx[7] ^ bufrx[6] 
		^ bufrx[5] ^ bufrx[4] ^ bufrx[3]
		^ bufrx[2] ^ bufrx[1] ^ bufrx[0];


always @(posedge CLK or posedge reset)
begin
	if (reset)
		begin
		bufrx <= 0;
		datorx <= 0;
		err_paridad <= 0;
		err_frame <= 0;
		end
	else casex ({fin_rx, samples})
		2'bx1: begin
			bufrx <= bufrx >> 1;
			bufrx[9] <= RxDs;
		end

		2'b1x: begin
			datorx <= bufrx[7:0];

			casex ({ParityForced, Parity, ParityEnable})
			3'bxx0: err_paridad <= 0; // No parity

			3'b001: err_paridad <= ~iparity; // ODD  parity
			3'b011: err_paridad <=  iparity; // EVEN parity
			3'b101: err_paridad <= 1; // forced parity
			3'b111: err_paridad <= 0; // forced parity

			endcase

			err_frame <= !bufrx[9];
		end
	endcase
end

endmodule





//............................................................
// BUFFER TRANSMITER
//............................................................
 
 
module pc_bufftx (reset, CLK, carga, load_shift, enable, datotx, TxD,
		ParityEnable, Parity, ParityForced);

input		reset;		// Reset (H). Lo pone a UNOS.
input		CLK;		// Reloj del sistema
input		carga;		// Carga (H) nuevo dato
input		load_shift;	// Carga(H)/Desplaza(L) dato en reg P-S
input		enable;		// Habilitacion (H) del reg P-S
input	[7:0]	datotx;		// Dato para transmitir
output		TxD;	 	// Salida 
input		ParityEnable;	// Habilitacion de paridad 
input		Parity;		// Paridad
input		ParityForced;	// Valor de paridad forzada.


// Variables
//.............................................................

reg  	[7:0]	dato_tx;	// Registro del dato para transmitir
reg     [10:0]  buftx;		// Registro paralelo-serie del Transmisor
				// buftx[8:1]=dato, buftx[9]=paridad 
				// buftx[0]=arranque, parada 
wire		TxD;	 	// Salida
wire		iparity;	// Evaluacion de la paridad
//.............................................................

// iparity is 1 when number of ones is odd.

assign iparity =   dato_tx[7] ^ dato_tx[6]
		 ^ dato_tx[5] ^ dato_tx[4]
		 ^ dato_tx[3] ^ dato_tx[2]
		 ^ dato_tx[1] ^ dato_tx[0];


always @(posedge CLK or posedge reset)
begin
	if (reset)
		begin
		dato_tx <= 0;
		buftx[10:0] <= 11'h7FF;
		end
	else casex ({load_shift, enable, carga})
		3'bxx1: begin
				dato_tx <= datotx;
			end

		3'b01x: begin
				buftx[9:0] <= buftx[10:1];
				buftx[10] <= 1;
			end

		3'b11x: begin
				buftx[8:1] <= dato_tx;
				buftx[0] <= 0;

				casex ({ParityForced, Parity, ParityEnable})
				3'bxx0: buftx[9] <= 1; // No parity

				3'b001: buftx[9] <= ~iparity; // ODD  parity
				3'b011: buftx[9] <=  iparity; // EVEN parity
				3'b101: buftx[9] <= 1; // forced parity
				3'b111: buftx[9] <= 0; // forced parity

				endcase
				
			end

	endcase
end

assign 	TxD = buftx[0];

endmodule






//............................................................
// RECEIVER CONTROL
//............................................................
 
module pc_ctrl_rx (reset, CLK, samples, rx_lleno);

input	reset;		// Reset (H)
input	CLK;		// Reloj del sistema
input	samples;	// Impulso de muestreo de RxDs conformado 
output	rx_lleno;	// Indica (H) final de la recepcion de un caracter 

// Variables
//.............................................................

reg		rx_lleno;	// Indica (H) final de la recepcion de un caracter
reg	[3:0]	cont_rx;// Contador de bits recibidos 

//.............................................................

always @(posedge CLK or posedge reset)
begin
	if (reset)
		begin
		rx_lleno <= 0;
		cont_rx <= 0;
		end
	else if (samples)
			begin
			cont_rx <= cont_rx + 1;
			if (cont_rx==10)
				begin
				rx_lleno <= 1;
				cont_rx <= 0;
                                end
			else
				rx_lleno <= 0;
			end
		else
			rx_lleno <= 0;
end

endmodule


//............................................................
// TRANSMMITER CONTROL
//............................................................
 
module pc_ctrl_tx_pc (reset, CLK, carga, enable, load_shift,
		 tx_empty, txt_empty);

input	reset;		// Reset (H)
input	CLK;		// Reloj del sistema
input	carga;		// Carga (H) de un nuevo dato
input	enable;		// Habilitacion (H) 
output  load_shift;     // Carga(H)/Desplaza(L) en reg P-S
output	tx_empty;	// Indica (H) TX vacio (PP) 
output	txt_empty;	// Indica (H) TX totalmente vacio (PP y PS) 

// Variables
//.............................................................

wire		load_shift;     // Carga(H)/Desplaza(L) en reg P-S
reg		tx_empty;       // Indica (H) buffer de tx vacio (PP)
wire		txt_empty;	// Indica (H) TX vacio (PP y PS)
reg		tx_on;		// Indica (H) transmision activa
reg	[3:0]	cont_tx;	// Contador de bits transmitidos 

//.............................................................

always @(posedge CLK or posedge reset)
begin
	if (reset)
		begin
		tx_empty <= 1;
		tx_on <= 0;
		cont_tx <= 0;
		end
	else 	begin
                casex ({carga, load_shift})
                2'b00:  tx_empty <= tx_empty;
                2'b01:  tx_empty <= 1;
                2'b1x:  tx_empty <= 0;
                endcase

 
                if (enable)
                        if(tx_on) begin
                                cont_tx <= cont_tx + 1;
                                if (cont_tx==10) begin
                                        cont_tx <= 0;
					tx_on <= 0;
				end
                                else 
					tx_on <= 1;
                        end
                        else if(load_shift)
                                tx_on <= 1;
 
 
        end
 
end
 
assign load_shift = (cont_tx == 0) & enable & !tx_empty & !tx_on;
assign txt_empty = tx_empty & !tx_on;

endmodule

//............................................................
// ERROR AND READY CONTROL
//
//............................................................
 
module pc_dato_rdy (reset, CLK, rx_lleno, borrar_rdy, borrar_err,
		dato_rdy, err_overrun);

input	reset;		// Reset (H)
input	CLK;		// Reloj del sistema
input	rx_lleno;	// Indica dato completo en el receptor
input	borrar_rdy;	// Indica lectura de DATO_RX
input	borrar_err;	// Indica lectura de STATUS_RX
output	dato_rdy;	// Indica (H) final de la recepcion de un caracter 
output	err_overrun;	// Error de rebosamiento

// Variables
//.............................................................

reg		dato_rdy;// Indica (H) final de la recepcion de un caracter
reg		err_overrun;	// Error de rebosamiento

//.............................................................

always @(posedge CLK or posedge reset)
begin
	if (reset)
		begin
		dato_rdy <= 0;
		err_overrun <= 0;
		end
	else begin
		if (rx_lleno)
			dato_rdy <= 1;
		else if (borrar_rdy)
			dato_rdy <= 0;

		if (rx_lleno & dato_rdy)
			err_overrun <= 1;
		else if(borrar_err)
			err_overrun <= 0;
	end
			
end
endmodule


//............................................................
// DIVIDER BY 27
//............................................................
 
module pc_div27 (reset, CLK, clk_in, clk_out);

input	reset;		// Reset (H)
input	CLK;		// Reloj del sistema
input	clk_in;		// Frecuencia de entrada 
output	clk_out;	// Frecuencia de salida

// Variables
//.............................................................

reg	[5:0]	div27;	// Registro para dividir la frecuencia de
			// muestreo y obtener la de transmision


always @(posedge CLK or posedge reset)
begin
	if (reset)
		div27 <= 0;
	else if (clk_in)
		if (div27==26)
			div27 <= 0;
		else
			div27 <= div27 + 1;
end

assign clk_out = (div27==26)?1:0;

endmodule


//............................................................
// DIVIDER BY 16
//............................................................
 
module pc_div16 (reset, CLK, clk_in, clk_out);

input	reset;		// Reset (H)
input	CLK;		// Reloj del sistema
input	clk_in;		// Frecuencia de entrada 
output	clk_out;	// Frecuencia de salida

// Variables
//.............................................................

reg	[3:0]	div16;	// Registro para dividir la frecuencia de
			// muestreo y obtener la de transmision


always @(posedge CLK or posedge reset)
begin
	if (reset)
		div16 <= 0;
	else if (clk_in)
		if (div16==15)
			div16 <= 0;
		else
			div16 <= div16 + 1;
end

assign clk_out = (div16==15)?1:0;

endmodule




//............................................................
// PROGRAMMABLE DIVIDER 
//............................................................
 
module pc_div_ms (reset, CLK, clkin, cte_div, carga_div_low, carga_div_high, clk_out);

input		reset;	// Reset (H)
input		CLK;	// Reloj del sistema
input		clkin;	// Reloj escalado
input	   [7:0]	cte_div;// Frecuencia de entrada 
input    carga_div_low; // Senal carga de la cte_div
input    carga_div_high; // Senal carga de la cte_div

output		clk_out;// Frecuencia de salida

// Variables
//.............................................................

reg	[15:0]	div;	// Contador del divisor 
reg	[15:0]	k_div;	// Factor de division
reg		clk_out;// Frecuencia de salida


always @(posedge CLK or posedge reset)
begin
	if (reset)
	  begin
		div   <= 0;
		k_div <= 1;
	  end
	else begin
		if (carga_div_low)                      
			k_div[7:0] <= cte_div;
      if (carga_div_high)
			k_div[15:8] <= cte_div;
      if (carga_div_low | carga_div_high) div<=0;
		if (div==k_div) begin
			 div <= 0;
			 clk_out <= 1;
		end
		else begin
			if (clkin)
			  begin
			    div <= div + 1;
			    clk_out <= 0;
			  end
		end
	end
end

endmodule


module pc_ier  (reset, CLK, 
             carga_IER, 
             data_in,
	     dato_rdy, tx_empty, modem_int, 
             nIRQ,
	     IER); 

input		reset;
input		CLK;
input		carga_IER;
input	[7:0]	data_in;

input		dato_rdy;
input		tx_empty;
input		modem_int;
output		nIRQ;		// Data Terminal Ready

output	[3:0]	IER;
reg	[3:0]	IER;
		
assign nIRQ= ~|(IER & {modem_int, dato_rdy, tx_empty, dato_rdy});

always @(posedge CLK)
begin
  if (reset)
	begin
          IER<=4'b0;
	end
  else

	if (carga_IER)
		begin
                  IER<=data_in[3:0];
		end
	
end



endmodule



//............................................................
// ARM INTERFACE 
//............................................................


// ADD  nRW   FUNCTION
// 000   0    Rx Register. 
// 000   1    TX Register. If enabled, LSB Divisor Latch.
// 
// 001   0    NA
// 001   1    Interrupt Enable Register. If enabled, MSB Divisor Latch.
// 
// 010   0    Interrupt Status Register.
// 010   1    FIFO Control Register (Not Implemented).
// 
// 011   0    NA
// 011   1    Line Control Register
// 
// 100   0    NA
// 100   1    Modem Control Register
// 
// 
// 101   0    Line Status Register
// 101   1    NA
// 
// 110   0    Modem Status Register
// 110   1    NA
// 
// 111  01    Scratch Pad Register
// 

module pc_if_arm_pc (reset, CLK, data_in, data_out, add, nRW, CS, 
		borrar_rdy, borrar_err, carga, carga_div_low, carga_div_high, 
      carga_IER, carga_MCR, carga_LCR, carga_MSR, carga_ISR,              
      BaudSelect, dato_rx, modem,  ISR, LSR, LCR);

input	 reset;					// Reset (H)
input	 CLK;						// Reloj del sistema
input	 [7:0]	data_in;		// Bus de datos de entrada
output [7:0]	data_out;	// Bus de datos de salida
input	 [2:0]	add;			// Bus de direcciones
input	 nRW;						// senal de lectura del ARM
input	 CS;						// senal de seleccion de la UART
input	 [7:0]	dato_rx;		// Dato recibido 
output borrar_rdy;     		// Borrar dato_rdy tras leer DATO_RX
output borrar_err;     		// Borrar err_over tras leer STATUS_RX
output carga;					// Senal carga 
output carga_div_low;		// Senal carga de la cte_div (byte bajo)
output carga_div_high;		// Senal carga de la cte_div (byte alto)
output carga_IER;				// senal carga IER
output carga_MCR;				// senal carga MCR
output carga_LCR;				// senal carga LCR
output carga_MSR;				// senal carga MSR (para codificacion delta)
output carga_ISR;				// senal carga de ISR
input	 BaudSelect;			// seleccion de modo de acceso
input	 [7:0]	modem;		// modem bus
input	 [3:0]	ISR;			// Interrupt Status Register
input	 [7:0]	LSR;			// Line Status Register 
input	 [7:0]	LCR;			// Line Control Register 


// Variables
//.............................................................

reg	[7:0]	data_out;		// Bus de datos de salida
reg		carga;				// Senal carga 
reg		carga_div_low;		// Senal carga de la cte_div (byte bajo)
reg		carga_div_high;	// Senal carga de la cte_div (byte alto)
reg		carga_IER;			// senal carga IER
reg		carga_MCR;			// senal carga MCR
reg		carga_LCR;			// senal carga LCR
reg		carga_MSR;			// senal lectura del MSR (para puesta a 0)
reg		carga_ISR;			// senal recarga del ISR
reg		borrar_rdy;  	   // Borrar dato_rdy tras leer DATO_RX
reg		borrar_err;    	// Borrar err_over tras leer STATUS_RX
reg	[7:0]   dato_tx;     // Dato para transmitir



//.............................................................



always @(nRW or CS or add or dato_rx  or LSR or ISR or LCR or modem or BaudSelect)
    if (CS)
      begin
		casex ({nRW, add})
			6'b0000: begin	//Lectura de DATO_RX
				data_out       <= dato_rx;
				borrar_rdy     <= 1;	
				borrar_err     <= 0;	
				carga          <= 0;
				carga_div_low  <= 0;
            carga_div_high <= 0;
            carga_IER      <= 0;
				carga_MCR      <= 0;
				carga_LCR      <= 0;
				carga_MSR      <= 0;
				carga_ISR      <= 0;
				end

			6'b1000: begin //Escritura Baud_Rate
            if (BaudSelect)
            begin
              carga_div_low <= 1;
              carga         <= 0;
            end
            else //Escritura de DATO_TX
            begin
              carga_div_low <= 0;
              carga         <= 1;
            end 
            carga_div_high  <= 0;
            carga_IER       <= 0;
				carga_MCR       <= 0;
				carga_LCR       <= 0;
				carga_MSR       <= 0;
				carga_ISR       <= 0;
 				data_out        <= dato_rx;
				borrar_rdy      <= 0;	
				borrar_err      <= 0;	
				end

			6'b1001: begin //Escritura Baud_Rate
				if (BaudSelect)
					begin
						carga_div_high <= 1;
						carga_IER      <= 0;
					end
				else //Escritura de IER
					begin
						carga_div_high <= 0;
						carga_IER      <= 1;
					end 
			   carga         <= 0;
			   carga_div_low <= 0;
				carga_MCR     <= 0;
				carga_LCR     <= 0;
				carga_MSR     <= 0;
				carga_ISR     <= 0;
 				data_out      <= dato_rx;
				borrar_rdy    <= 0;	
				borrar_err    <= 0;	
				end


			6'b0010: begin //Lectura ISR
             carga          <= 0;
             carga_div_low  <= 0;
             carga_div_high <= 0;  
 				 data_out       <= ISR;
             carga_IER      <= 0;
				 carga_MCR      <= 0; 
				 carga_LCR      <= 0;
				 carga_MSR      <= 0;
				 carga_ISR      <= 1;
				 borrar_rdy     <= 0;	
				 borrar_err     <= 0;	
				 end


			6'b1010: begin //Escritura FCR (no implementado)
             carga          <= 0;
             carga_div_low  <= 0;
             carga_div_high <= 0;  
             carga_IER      <= 0;
				 carga_MCR      <= 0; 
				 carga_LCR      <= 0;
				 carga_MSR      <= 0;
				 carga_ISR      <= 0;
 				 data_out       <= dato_rx;
				 borrar_rdy     <= 0;	
				 borrar_err     <= 0;	
				 end


			6'b0011: begin //Lectura LCR 
             carga          <= 0;
             carga_div_low  <= 0;
             carga_div_high <= 0;  
             carga_IER      <= 0;
				 carga_MCR      <= 0; 
				 carga_LCR      <= 0;
				 carga_MSR      <= 0;
				 carga_ISR      <= 0;
 				 data_out       <= LCR;
				 borrar_rdy     <= 0;	
				 borrar_err     <= 0;	
				 end


			6'b1011: begin //Escritura LCR 
             carga          <= 0;
             carga_div_low  <= 0;
             carga_div_high <= 0;  
             carga_IER      <= 0;
				 carga_MCR      <= 0; 
				 carga_LCR      <= 1;
				 carga_MSR      <= 0;
				 carga_ISR      <= 0;
 				 data_out       <= dato_rx;
				 borrar_rdy     <= 0;	
				 borrar_err     <= 0;	
				 end


			6'b1100: begin	//Escritura de MCR
             carga          <= 0;
             carga_div_low  <= 0;
             carga_div_high <= 0;  
             carga_IER      <= 0;
				 carga_MCR      <= 1; 
				 carga_LCR      <= 0;
				 carga_MSR      <= 0;
				 carga_ISR      <= 0;
 				 data_out       <= dato_rx;
				 borrar_rdy     <= 0;	
				 borrar_err     <= 0;	
				 end

			6'b0101: begin	//Lectura del LSR
             carga<=0;
             carga_div_low<=0;
             carga_div_high<=0; 
             carga_IER<=0;
				 carga_MCR<=0; 
				 carga_LCR<=0;
				 carga_MSR<=0;
				 carga_ISR<=0;
             data_out <= LSR;
				 borrar_rdy <= 0;	
				 borrar_err <= 0;	
				 end


			6'b0110: begin	//Lectura del MSR
             carga<=0;
             carga_div_low<=0;
             carga_div_high<=0;
             carga_IER<=0;
				 carga_MCR<=0; 
				 carga_LCR<=0;
				 carga_MSR<=1;
				 carga_ISR<=0;
             data_out <= modem;  
				 borrar_rdy <= 0;	
				 borrar_err <= 0;	
				 end
/*
			6'b0111: begin	
                                 carga<=0;
                                 carga_div_low<=0;
                                 carga_div_high<=0;  
                                 carga_IER<=0;
				 carga_MCR<=0; 
				 carga_LCR<=0;
				 carga_ISR<=0;
 				 data_out <= dato_rx; 
				 borrar_rdy <= 0;	
				 borrar_err <= 0;	
				 end

			6'b1111: begin	
                                 carga<=0;
                                 carga_div_low<=0;
                                 carga_div_high<=0;  
                                 carga_IER<=0;
				 carga_MCR<=0; 
				 carga_LCR<=0;
				 carga_ISR<=0;
 				 data_out <= dato_rx;
				 borrar_rdy <= 0;	
				 borrar_err <= 0;	
				 end
*/

           default:
             begin
             carga<=0;
             carga_div_low<=0;
             carga_div_high<=0;  
             carga_IER<=0;
				 carga_MCR<=0; 
				 carga_LCR<=0;
				 carga_MSR<=0;
				 carga_ISR<=0;
 				 data_out <= dato_rx; 
				 borrar_rdy <= 0;	
				 borrar_err <= 0;	
             end

		endcase
              end
           else
		begin
             carga<=0;
             carga_div_low<=0;
             carga_div_high<=0;  
             carga_IER<=0;
				 carga_MCR<=0; 
				 carga_LCR<=0;
				 carga_MSR<=0;
				 carga_ISR<=0;
 				 data_out <= dato_rx; 
				 borrar_rdy <= 0;	
				 borrar_err <= 0;	
		end
endmodule



//............................................................
// RxD INTERFACE
//............................................................
 
module pc_ifrxd (reset, CLK, clkms, RxD, RxDs);

input	reset;		// Reset (H)
input	CLK;		// Reloj del sistema
input	clkms;  	// Reloj de muestreo sincronizado (el nivel
                        // alto dura un periodo de reloj)
input	RxD;		// Linea de recepcion de datos
output	RxDs;		// RxD sincronizada y limpia

// Variables
//.............................................................

reg	[2:0]	ifrxd;	// Registro interfaz de la linea RxD


//.............................................................

always @(posedge CLK or posedge reset)
begin
	if (reset)
		ifrxd <= 3'b111;
	else if (clkms)
		begin
		if ((ifrxd[0]==ifrxd[2]) & (ifrxd[0]!=ifrxd[1]))
			ifrxd[2] <= ifrxd[0];
		else
			ifrxd[2] <= ifrxd[1];
		ifrxd[1] <= ifrxd[0];
		ifrxd[0] <= RxD;
		end
end

assign RxDs = ifrxd[2];

endmodule
module pc_isr  (reset, CLK, 
             carga_ISR, 
	     err_paridad, err_frame, err_overrun, 
	     dato_rdy, tx_empty, txt_empty, modem_int, 
             ISR, LSR, IER); 

input		reset;
input		CLK;
input		carga_ISR;
input		err_paridad;
input		err_frame;
input		err_overrun;
input		dato_rdy;
input		tx_empty;
input		txt_empty;
input		modem_int;

output	[3:0]	ISR;		// Interrupt Status Register
output	[7:0]	LSR;		// Line Status Register
input	[3:0]	IER;		// Interrupt Enable Register

reg	[3:0]	ISR;
reg	[3:0]	ISRt;

reg		mask_error;
reg		mask_dato_rdy;
reg		mask_err_overrun;
reg		mask_tx_empty;
reg		mask_modem_int;
reg		carga_ISRd;
reg		aux1;

wire	error, errorm, dato_rdym, err_overrunm, tx_emptym, modem_intm;


assign errorm=		error 		& 	mask_error	& IER[1];
assign dato_rdym=	dato_rdy 	& 	mask_dato_rdy	& IER[0];
assign err_overrunm=	err_overrun 	& 	mask_err_overrun;
assign tx_emptym=	tx_empty 	& 	mask_tx_empty	& IER[1];
assign modem_intm=	modem_int 	& 	mask_modem_int	& IER[3];


always @(errorm or dato_rdym or tx_emptym or err_overrunm or modem_intm)
begin
  if (errorm)	ISRt=4'b0110;
  else if (dato_rdym)    ISRt=4'b0100;
  	//else if (err_overrunm) ISRt=4'b1100;
  		else if (tx_emptym)    ISRt=4'b0010;
  			else if (modem_intm)  ISRt=4'b0000;
  				else          ISRt=4'b0001;
end

assign error  = err_paridad | err_frame | err_overrun;
assign LSR = {error, txt_empty, tx_empty, 1'b0, err_frame, err_paridad, err_overrun, dato_rdy};
		
always @(posedge CLK)
begin
  if (reset)
	begin
          	ISR<=4'b0001;
		mask_error<=1;
		mask_dato_rdy<=1;
		mask_err_overrun<=1;
		mask_tx_empty<=1;
		mask_modem_int<=1;
		carga_ISRd<=0;
		aux1<=1'b0;
	end
  else
    begin

        if (mask_error)
           begin
             if ((ISR==4'b0110) & carga_ISRd) mask_error<=0;
           end
        else
           if (error) mask_error<=1;

        if (mask_dato_rdy)
           begin
             if ((ISR==4'b0100) & carga_ISRd) mask_dato_rdy<=0;
           end
        else
           if (dato_rdy) mask_dato_rdy<=1;

        if (mask_err_overrun)
           begin
             if ((ISR==4'b1100) & carga_ISRd) mask_err_overrun<=0;
           end
        else
           if (err_overrun) mask_err_overrun<=1;

        if (mask_tx_empty)
           begin
             if ((ISR==4'b0010) & carga_ISRd) mask_tx_empty<=0;
           end
        else
           if (tx_empty) mask_tx_empty<=1;

        if (mask_modem_int)
           begin
             if ((ISR==4'b0000) & carga_ISRd) mask_modem_int<=0;
           end
        else
           if (modem_int) mask_modem_int<=1;

        ISR<=ISRt;
	aux1<=carga_ISR; //Espera a que se deseleccione el carga_ISR para modificar mascaras.
	if (~carga_ISR) 
		begin
			carga_ISRd<=aux1;
			aux1<=1'b0;
		end

    end	
end


endmodule


module pc_lcr  (reset, CLK, LCR, 
             carga_LCR, 
             data_in, 
             WordLength, Stop, ParityEnable, Parity,
	     ParityForced, Break, BaudSelect); 

input		reset;
input		CLK;
output	[7:0]	LCR;
input		carga_LCR;
input	[7:0]	data_in;

output	[1:0]	WordLength;
output		Stop;
output		ParityEnable;
output		Parity;
output		ParityForced;
output		Break;
output		BaudSelect;		

reg 	[7:0]	LCR;	


assign WordLength={LCR[1],LCR[0]};
assign Stop=LCR[2];
assign ParityEnable=LCR[3];
assign Parity=LCR[4];
assign ParityForced=LCR[5];
assign Break=LCR[6];
assign BaudSelect=LCR[7];



always @(posedge CLK)
begin
  if (reset)
	begin
          LCR<=8'b0;	//Ver hoja de datos de ST16C552.
			//Esta es la situacion en reset.
	end
  else

	if (carga_LCR)
		begin
			LCR<=data_in;
		end
	

end




endmodule


//module pc_modem(reset, CLK, 
//             carga_MCR, carga_MSR,
//             data_in, modem,
//             CD, RI, DSR, CTS, DTR, RTS, modem_int); 
//
//input		reset;
//input		CLK;
//input		carga_MCR;
//input		carga_MSR;
//input	[7:0]	data_in;
//output	[7:0]	modem;
//input		CD;		// Carrier Detect
//input		RI;		// Ring Indicator
//input		DSR;		// Data Send Ready
//input		CTS;		// Clear To Send
//output		RTS;		// Request To Send
//output		DTR;		// Data Terminal Ready
//output		modem_int;
//
//reg 		RTS;
//reg		DTR;
//
//reg		dCD,  sCD;
//reg		dRI,  sRI;
//reg		dDSR, sDSR;
//reg		dCTS, sCTS;
//wire	[7:0]	modem;
//reg		modem_int;
//
//
//assign modem ={~sCD,      ~sRI,      ~sDSR,       ~sCTS, 
//                dCD ^ sCD, dRI ^ sRI, dDSR ^ sDSR, dCTS ^ sCTS};
//
//
//always @(posedge CLK)
//begin
//  if (reset)
//	begin
//		dCD<=1;  sCD<=1;
//		dRI<=1;  sRI<=1; 
//		dDSR<=1; sDSR<=1;
//		dCTS<=1; sCTS<=1;
//		DTR<=1;
//		RTS<=1;		
//		modem_int<=0;
//	end
//  else
//    begin
//	sCD<=~CD;
//	sRI<=~RI;
//	sDSR<=~DSR;
//	sCTS<=~CTS;
//
//	modem_int<=modem_int | (|modem[3:0]);     
//
//	if (carga_MCR)
//		begin
//			RTS<=~data_in[0];
//			DTR<=~data_in[1];
//		end
//	
//	else
//		if (carga_MSR)
//			begin
//				dCD<=~CD;
//				dRI<=~RI;
//				dDSR<=~DSR;
//				dCTS<=~CTS;
//				modem_int<=0; //Limpieza de la interrupcion
//			end
//
//
//   end
//
//end
//
//
//
//
//endmodule
//

//............................................................
// RxDs SAMPLER
//............................................................
 
module pc_muestreo (reset, CLK, rst_muestreo, clkms, RxDs, sample);

input	reset;		// Reset (H)
input	CLK;		// Reloj del sistema
input	rst_muestreo;	// Reset (H) del circuito de muestreo 
input	clkms;		// Reloj de muestreo sincronizado (el nivel
                        // alto dura un periodo de reloj)
input	RxDs;		// Senal RxD sincronizada y limpia
output	sample; 	// Impulso de muestreo de la senal RxDs

// Variables
//.............................................................

reg	[3:0]	cont_m;	// Contador del circuito de muestreo 
reg		flag_rx;// Indicador de recepcion en curso
reg		sample;	// Impulso de muestreo de la senal RxDs


//.............................................................

always @(posedge CLK or posedge reset)
begin
	if (reset)
		begin
		cont_m <= 4'b0000;
		sample <= 0;
		flag_rx <= 0;
		end
	else if (rst_muestreo)
		begin
		 cont_m <= 4'b0000;
		 sample <= 0;
		 flag_rx <= 0;
                end
	else if (clkms)
		if ( (flag_rx==0) & (RxDs==0) ) // Arranque
			flag_rx <= 1;	// Recepcion en curso
		else if (flag_rx)
			begin
			cont_m <= cont_m + 1;
			if (cont_m==4'b0110)
				sample <= 1;
			else
				sample <= 0;
			end
end

endmodule



//............................................................
// TIMING CONTROL
//............................................................
 
module pc_pulso (reset, CLK, dato_asyn, dato_syn);

input	reset;		// Reset (H)
input	CLK;		// Reloj del sistema
input	dato_asyn;	// Entrada de dato asincrona
output	dato_syn;	// Salida de dato conformada

// Variables
//.............................................................

reg		dff;	// Registro del sincronizador

always @(posedge CLK or posedge reset)
begin
	if (reset)
		dff <= 0;
	else
		dff <= dato_asyn;
end

assign dato_syn = !dff & dato_asyn;

endmodule
