<?php
# Matus Remen (xremen01@stud.fit.vutbr.cz)
class Parser{
	private $xml;
	private $order;

	public function __construct($argc, $argv){
		$this->argc = $argc;
		$this->argv = $argv;
		$xml_header = '<?xml version="1.0" encoding="UTF-8" ?><program language="IPPcode22"></program>';
		$this->xml = new SimpleXMLElement($xml_header);
		$this->order = 1;
		$this->parse_prg_params($argc, $argv);
	}

	public function parse(){
		$line = explode('#', fgets(STDIN));
		if (!preg_match('/\.IPPcode22/', trim($line[0]))){
			exit(21);
		}
		while ($line = fgets(STDIN)){
			$line = explode('#', $line)[0];
			$line = trim($line);
			$splitted_line = preg_split('/(\s)+/', $line);
			$this->parse_ins($splitted_line);
		}
		echo $this->xml->asXML();
	}

	private function parse_ins($splitted_line){
		$line_ins = $splitted_line[0];
		if ($line_ins == ''){
			return;
		}
		$instruction = $this->xml->addChild('instruction');
		$instruction->addAttribute('order', $this->order);
		$instruction->addAttribute('opcode', strtoupper($line_ins));
		if (preg_match('/^(MOVE|INT2CHAR|STRLEN|TYPE|NOT)$/', $line_ins)){
			$this->check_params($splitted_line, 2, array('var', 'symb'));
		}
		else if (preg_match('/^(CREATEFRAME|PUSHFRAME|POPFRAME|RETURN|BREAK)$/', $line_ins)){
			$this->check_params($splitted_line, 0, array());
		}
		else if (preg_match('/^(DEFVAR|POPS)$/', $line_ins)){
			$this->check_params($splitted_line, 1, array('var'));
		}
		else if (preg_match('/^(CALL|LABEL|JUMP)$/i', $line_ins)){
			$this->check_params($splitted_line, 1, array('label'));
		}
		else if (preg_match('/^(PUSHS|WRITE|EXIT|DPRINT)$/', $line_ins)){
			$this->check_params($splitted_line, 1, array('symb'));
		}
		else if (preg_match('/^(ADD|SUB|MUL|IDIV|LT|GT|EQ|AND|OR|NOT|STRI2INT|CONCAT|GETCHAR|SETCHAR)$/', $line_ins)){
			$this->check_params($splitted_line, 3, array('var', 'symb', 'symb'));
		}
		else if (preg_match('/^(READ)$/', $line_ins)){
			$this->check_params($splitted_line, 2, array('var', 'type'));
		}
		else if (preg_match('/^(JUMPIFEQ|JUMPIFNEQ)$/', $line_ins)){
			$this->check_params($splitted_line, 3, array('label', 'symb', 'symb'));
		}
		else {
			exit(22);
		}
		$this->order++;
	}

	private function check_params($splitted_line, $param_count, $param_type){
		if (count($splitted_line)-1 != $param_count){
			exit(23);
		}
		for ($i = 1; $i < count($splitted_line); $i++){
			$re = '';
			switch ($param_type[$i-1]){
				case 'var':
					$re = '/^(GF|LF|TF)@[_\-$%*?!&@\w\d]+$/';
					break;
				case 'label':
					$re = '/^[_\-$%*?!&\w\d]+$/';
					break;
				case 'type':
					$re = '/^(int|string|bool)$/';
					break;
				case 'symb':
					$re = '/^(string@(\\\d{3}|\S)*)$|';
					$re .= '^(int@-?\d*)$|';
					$re .= '^(bool@(true|false)$|';
					$re .= '^(nil@nil))$/';
					if (preg_match($re, $splitted_line[$i])){
						$arg = $this->xml->instruction[$this->order-1]->addChild("arg$i", htmlspecialchars(explode('@', $splitted_line[$i], 2)[1]));
						$arg->addAttribute('type', explode('@', $splitted_line[$i])[0]);
						continue 2;
					} else {
						$re = '/^(GF|LF|TF)@[_\-$%*?!&@\w\d]+$/';
						$param_type[$i-1] = 'var';
					}
					break;
				default:
					exit(99);
			}
			if (preg_match($re, $splitted_line[$i])){
				$arg = $this->xml->instruction[$this->order-1]->addChild("arg$i", htmlspecialchars($splitted_line[$i]));
				$arg->addAttribute('type', $param_type[$i-1]);
			} else {
				exit(23);
			}
		}
	}

	private function parse_prg_params($argc, $argv){
		if ($argc == 1){
			return;
		}
		if ($argc == 2 && $argv[1] == '--help'){
			$this->print_help();
			exit(0);
		}
	}

	private function print_help(){
		echo "IPPcode22 parser\n"
			."Usage: php8.1 parse.php < in_filestream > out_filestream\n"
			."Optional params:\n"
			."\t[--help] - print this message\n";
		exit(0);
	}

}

$parser = new Parser($argc, $argv);
$parser->parse();

