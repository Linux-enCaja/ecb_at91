library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity counter is
    Port ( clk 			: in  STD_LOGIC;
           led, led_lcd : out  STD_LOGIC);
end counter;

architecture Behavioral of counter is

signal count : integer range 0 to 48000000;


begin

 process(clk)
  begin
  if clk'event and clk='1' then
    if count < 24000000 then
	    led 		<= '1';
		 led_lcd <= '1';
	 elsif count < 48000000 then
	    led 		<= '0';
		 led_lcd <= '0';
	 else
	    count <= 0;
	 end if;
    count <= count +1;	 
  end if;
 end process;


end Behavioral;

