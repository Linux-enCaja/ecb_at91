library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity basic_io is	  port(
			-- Processor Interface
			A			  					: in		std_logic_vector(18 downto 1);
			D  		  					: inout	std_logic_vector(15 downto 0);
			nwe, nrd   					: in		std_logic;
			ncs2, ncs3 					: in		std_logic;
			rst							: in 		std_logic; 
			pc20, pc21 					: out		std_logic;
			-- LCD Interface
			LCD_D							: inout	std_logic_vector(15 downto 8);
			lcd_rs, lcd_cs, lcd_rd	: out		std_logic;
			lcd_rst, lcd_wr, lcd_led: out		std_logic;
			--SDRAM Interface
			SDD							: inout	std_logic_vector(15 downto 0);
			SDA							: in		std_logic_vector(12 downto 0);
			ba1, ba0, cke, sdclk		: out		std_logic;
			dqml, dqmh, cas, ras		: out		std_logic;
			sdwe, sdcs					: out		std_logic;
			--General pins
			 CLK							: in		std_logic;
			 LED							: buffer		std_logic
	  );
end basic_io;

architecture Behavioral of basic_io is


signal DO, DI : std_logic_vector(15 downto 0);

type state is (s0, s1, srd, srd1, swr, swr1);
signal current, nexts : state;

signal clk200 : std_logic;
signal dummy0, dummy1, dummy2  : std_logic;
signal nrd_sync, nwe_sync, ncs_sync  : std_logic;



signal WE :std_logic;


	component sram1
	port (
		A    : IN std_logic_VECTOR(3 downto 0);
		CLK  : IN std_logic;
		D    : IN std_logic_VECTOR(15 downto 0);
		WE   : IN std_logic;
		SPO  : OUT std_logic_VECTOR(15 downto 0));
	end component;

	COMPONENT dcm1
	PORT(
		CLKIN_IN : IN std_logic;          
		CLKFX_OUT : OUT std_logic;
		CLKIN_IBUFG_OUT : OUT std_logic;
		CLK0_OUT : OUT std_logic;
		LOCKED_OUT : OUT std_logic
		);
	END COMPONENT;
	
begin

--	D <= (others => 'Z');
	pc20 <= 'Z';  pc21 <= 'Z';

	LCD_D   <= (others => 'Z');
	lcd_rs  <= 'Z'; lcd_cs <= 'Z'; lcd_rd <= 'Z';
	lcd_rst <= 'Z';
	lcd_wr <= 'Z'; lcd_led <= 'Z';

	SDD  <= (others => 'Z');
	ba1  <= 'Z'; ba0  <= 'Z'; cke <= 'Z'; sdclk <= 'Z';
	dqml <= 'Z'; dqmh <= 'Z'; cas <= 'Z'; ras   <= 'Z';
	sdwe <= 'Z'; sdcs <= 'Z';
	led  <= 'Z';



   LCD_D(15) <= clk200;
   LCD_D(13) <= ncs_sync;
   LCD_D(11) <= nrd_sync;
   LCD_D(9)  <= nwe_sync;
	
   LCD_D(14) <= 'Z'; LCD_D(12) <= 'Z'; LCD_D(10) <= 'Z'; LCD_D(8) <= 'Z';

   clk200 <= clk;

--	dcm1_inst:  dcm1  port map ( clk, clk200, dummy0, dummy1, dummy2 );
	sram_innst: sram1 port map ( A(4 downto 1), clk200, DI, WE, DO );

  fsm_rw: process( nexts)
  begin
    case current is
	   when s0 =>					-- wait for ncs0
		  WE <= '0';
		  D  <= (others => 'Z');
		  if ncs_sync = '1' then
		    nexts <= s0;
		  else
		    nexts <= s1;
		  end if;
		  
	   when s1 =>					-- wait for nrd or nwe
		  WE <= '0';
	     if ncs_sync = '1' then
		    D       <= (others => 'Z');
		    nexts   <= s0;
		  else
		    if nrd_sync = '0' then
		      D       <= DO;
		      nexts   <= srd;
		    elsif nwe_sync = '0' then
		      nexts   <= swr1;
				DI 	  <= D;
				WE		  <= '1';
		      D       <= (others => 'Z');
		    else
			   D       <= (others => 'Z');
		      nexts   <= s1;
		    end if;
		  end if;
		  
		when srd =>
        D  <= DO;
		  WE <= '0';
	     if ncs_sync = '1' then
		    nexts   <= srd1;
		  else
		    nexts <= srd;
		  end if;


		when srd1 =>
		  WE <= '0';
        D  <= DO;
		  nexts   <= s0;
		  
		when swr =>
		  WE <= '1';
		  DI <= D;
		  D     <= (others => 'Z');
		  if nwe_sync = '0' then
		    nexts <= swr;
		  else
		    nexts <= swr1;
		  end if;
		  
		when swr1 =>
		  WE <= '0';
		  D     <= (others => 'Z');
		  nexts <= s0;

		when others =>
		  WE <= '0';
		  nexts <= s0;
		  D  <= (others => 'Z');
	 end case;
  end process fsm_RW;
  
  
  fsm_clk: process(clk200, nexts)
  begin
    if clk200'event and clk200='0' then
	   current <= nexts;
		nrd_sync <= nrd;
		nwe_sync <= nwe;
		ncs_sync <= ncs2;
	 end if;
  end process fsm_clk;

end Behavioral;

