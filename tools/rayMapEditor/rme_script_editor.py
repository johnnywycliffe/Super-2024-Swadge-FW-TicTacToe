import re
from enum import Enum


class ifOpType(Enum):
    SHOOT_OBJS = 0,
    SHOOT_WALLS = 1,
    KILL = 2,
    ENTER = 3,
    GET = 4,
    TOUCH = 5,
    BUTTON_PRESSED = 6,
    TIME_ELAPSED = 7


class thenOpType(Enum):
    OPEN = 8,
    CLOSE = 9,
    SPAWN = 10,
    DESPAWN = 11,
    DIALOG = 12,
    WARP = 13,
    WIN = 14


class orderType(Enum):
    IN_ORDER = 0,
    ANY_ORDER = 1


class rme_script:
    def __init__(self) -> None:
        self.ifOp: ifOpType = None
        self.ifArgs = {}
        self.thenOp: thenOpType = None
        self.thenArgs = {}
        pass

    def parseArgs(self, args: str) -> list[str]:
        # Args are in the form (a;b;c)
        result = re.match(r'\s*\((.*)\)\s*', args)
        if result:
            return [s.strip() for s in re.split(r';', result.group(1).strip())]
        return None

    def parseArray(self, array: str) -> list[str]:
        # Arrays are in the form [a,b,c]
        result = re.match(r'\s*\[(.*)\]\s*', array)
        if result:
            return [s.strip() for s in re.split(r',', result.group(1).strip())]
        return None

    def parseInt(self, integer: str) -> int:
        # Integers are integers
        return int(integer.strip())

    def parseOrder(self, order: str) -> orderType:
        # Order is either IN_ORDER or ANY_ORDER
        for type in orderType.__members__.items():
            if order == type[0]:
                return type[1]
        return None

    def parseCell(self, cell: str) -> list[int]:
        # Cells are in the form {a.b}
        result = re.match(r'\s*{\s*(\d+)\s*\.\s*(\d+)\s*}\s*', cell)
        if result:
            return [int(result.group(1)), int(result.group(2))]
        return None

    def parseText(self, text: str) -> str:
        # Text is a string, not quoted
        return text.strip()

    def isListNotValid(self, array: list) -> bool:
        return (array is None) or (0 == len(array)) or (None in array)

    def toString(self) -> str:

        # Stringify the if args
        ifArgArray = []
        if 'cell' in self.ifArgs.keys():
            ifArgArray.append('{' + str(self.ifArgs['cell'][0]) + '.' + str(self.ifArgs['cell'][1]) + '}')
        if 'ids' in self.ifArgs.keys():
            ifArgArray.append('[' + ', '.join(str(e) for e in self.ifArgs['ids']) + ']')
        if 'cells' in self.ifArgs.keys():
            ifArgArray.append('[' + ', '.join('{' + str(e[0]) + '.' + str(e[1]) + '}' for e in self.ifArgs['cells']) + ']')
        if 'order' in self.ifArgs.keys():
            ifArgArray.append(self.ifArgs['order'].name)
        if 'id' in self.ifArgs.keys():
            ifArgArray.append(str(self.ifArgs['id']))
        if 'btn' in self.ifArgs.keys():
            ifArgArray.append(str(self.ifArgs['btn']))
        if 'tMs' in self.ifArgs.keys():
            ifArgArray.append(str(self.ifArgs['tMs']))

        # Stringify the then args
        thenArgArray = []
        if 'cells' in self.thenArgs.keys():
            thenArgArray.append('[' + ', '.join('{' + str(e[0]) + '.' + str(e[1]) + '}' for e in self.thenArgs['cells']) + ']')
        if 'cell' in self.thenArgs.keys():
            thenArgArray.append('{' + str(self.thenArgs['cell'][0]) + '.' + str(self.thenArgs['cell'][1]) + '}')
        if 'ids' in self.thenArgs.keys():
            thenArgArray.append('[' + ', '.join(str(e) for e in self.thenArgs['ids']) + ']')
        if 'text' in self.thenArgs.keys():
            thenArgArray.append(self.thenArgs['text'])
        # TODO handle spawn operation

        # Stitch it all together
        return 'IF ' + self.ifOp.name + '(' + '; '.join(ifArgArray)+ ') THEN ' + self.thenOp.name + '(' + '; '.join(thenArgArray) + ')'


    def fromString(self, if_op: str, if_args: str, then_op: str, then_args: str) -> bool:
        try:
            # Split the args
            argParts = self.parseArgs(if_args)

            # Parse the IF part
            if ifOpType.SHOOT_OBJS.name == if_op:
                # Set the operation type
                self.ifOp = ifOpType.SHOOT_OBJS
                # Parse the args
                self.ifArgs['ids'] = [self.parseInt(
                    s) for s in self.parseArray(argParts[0])]
                self.ifArgs['order'] = self.parseOrder(argParts[1])
                # Validate the args
                if self.isListNotValid(self.ifArgs['ids']) or self.ifArgs['order'] is None:
                    return False

            elif ifOpType.SHOOT_WALLS.name == if_op:
                self.ifOp = ifOpType.SHOOT_WALLS
                # Parse the args
                self.ifArgs['cells'] = [self.parseCell(
                    s) for s in self.parseArray(argParts[0])]
                self.ifArgs['order'] = self.parseOrder(argParts[1])
                # Validate the args
                if self.isListNotValid(self.ifArgs['cells']) or self.ifArgs['order'] is None:
                    return False

            elif ifOpType.KILL.name == if_op:
                # Set the operation type
                self.ifOp = ifOpType.KILL
                # Parse the args
                self.ifArgs['ids'] = [self.parseInt(
                    s) for s in self.parseArray(argParts[0])]
                self.ifArgs['order'] = self.parseOrder(argParts[1])
                # Validate the args
                if self.isListNotValid(self.ifArgs['ids']) or self.ifArgs['order'] is None:
                    return False

            elif ifOpType.ENTER.name == if_op:
                self.ifOp = ifOpType.ENTER
                # Parse the args
                self.ifArgs['cell'] = self.parseCell(argParts[0])
                self.ifArgs['ids'] = [self.parseInt(
                    s) for s in self.parseArray(argParts[1])]
                # Validate the args
                if self.isListNotValid(self.ifArgs['ids']) or self.ifArgs['cell'] is None:
                    return False

            elif ifOpType.GET.name == if_op:
                self.ifOp = ifOpType.GET
                # Parse the args
                self.ifArgs['ids'] = [self.parseInt(
                    s) for s in self.parseArray(argParts[0])]
                # Validate the args
                if self.isListNotValid(self.ifArgs['ids']):
                    return False

            elif ifOpType.TOUCH.name == if_op:
                self.ifOp = ifOpType.TOUCH
                # Parse the args
                self.ifArgs['id'] = self.parseInt(argParts[0])
                # Validate the args
                if self.ifArgs['id'] is None:
                    return False

            elif ifOpType.BUTTON_PRESSED.name == if_op:
                self.ifOp = ifOpType.BUTTON_PRESSED
                # Parse the args
                self.ifArgs['btn'] = self.parseInt(argParts[0])
                # Validate the args
                if self.ifArgs['btn'] is None:
                    return False

            elif ifOpType.TIME_ELAPSED.name == if_op:
                self.ifOp = ifOpType.TIME_ELAPSED
                # Parse the arg
                self.ifArgs['tMs'] = self.parseInt(argParts[0])
                # Validate the args
                if self.ifArgs['tMs'] is None:
                    return False
            else:
                return False

            # Split the args
            argParts = self.parseArgs(then_args)

            # Parse the THEN part
            if thenOpType.OPEN.name == then_op:
                # Set the operation
                self.thenOp = thenOpType.OPEN
                # Parse the args
                self.thenArgs['cells'] = [self.parseCell(
                    s) for s in self.parseArray(argParts[0])]
                # Validate the args
                if self.isListNotValid(self.thenArgs['cells']):
                    return False

            elif thenOpType.CLOSE.name == then_op:
                # Set the operation
                self.thenOp = thenOpType.CLOSE
                # Parse the args
                self.thenArgs['cells'] = [self.parseCell(
                    s) for s in self.parseArray(argParts[0])]
                # Validate the args
                if self.isListNotValid(self.thenArgs['cells']):
                    return False

            elif thenOpType.SPAWN.name == then_op:
                self.thenOp = thenOpType.SPAWN
                # TODO parse SPAWN

            elif thenOpType.DESPAWN.name == then_op:
                self.thenOp = thenOpType.DESPAWN
                # Parse the args
                self.thenArgs['ids'] = [self.parseInt(
                    s) for s in self.parseArray(argParts[0])]
                # Validate the args
                if self.isListNotValid(self.thenArgs['ids']):
                    return False

            elif thenOpType.DIALOG.name == then_op:
                # Set the operation
                self.thenOp = thenOpType.DIALOG
                # Parse the args
                self.thenArgs['text'] = self.parseText(argParts[0])
                # Validate the args
                if self.thenArgs['text'] is None:
                    return False

            elif thenOpType.WARP.name == then_op:
                # Set the operation
                self.thenOp = thenOpType.WARP
                # Parse the args
                self.thenArgs['cell'] = self.parseCell(argParts[0])
                # Validate the args
                if self.thenArgs['cell'] is None:
                    return False

            elif thenOpType.WIN.name == then_op:
                # Set the operation
                self.thenOp = thenOpType.WIN
                # No arguments

            else:
                return False

            return True
        except:
            self.ifOp: ifOpType = None
            self.ifArgs = {}
            self.thenOp: thenOpType = None
            self.thenArgs = {}
            return False

    def toBytes(self) -> bytearray:
        bytes: bytearray = bytearray()

        # Append the IF operation
        bytes.append(self.ifOp.value[0])

        # Append the IF arguments, order matters
        if 'cell' in self.ifArgs.keys():
            bytes.append(self.ifArgs['cell'][0])
            bytes.append(self.ifArgs['cell'][1])
        if 'ids' in self.ifArgs.keys():
            bytes.append(len(self.ifArgs['ids']))
            for id in self.ifArgs['ids']:
                bytes.append(id)
        if 'cells' in self.ifArgs.keys():
            bytes.append(len(self.ifArgs['cells']))
            for cell in self.ifArgs['cells']:
                bytes.append(cell[0])
                bytes.append(cell[1])
        if 'order' in self.ifArgs.keys():
            if orderType.IN_ORDER == self.ifArgs['order']:
                bytes.append(0)
            elif orderType.ANY_ORDER == self.ifArgs['order']:
                bytes.append(1)
        if 'id' in self.ifArgs.keys():
            bytes.append(self.ifArgs['id'])
        if 'btn' in self.ifArgs.keys():
            bytes.append(self.ifArgs['btn']) # Should this be 8 bit?
        if 'tMs' in self.ifArgs.keys():
            bytes.append(self.ifArgs['tMs']) # Should this be 8 bit?

        # Append the ELSE operation
        bytes.append(self.thenOp.value[0])

        # Append the ELSE arguments, order matters
        if 'cells' in self.thenArgs.keys():
            bytes.append(len(self.thenArgs['cells']))
            for cell in self.thenArgs['cells']:
                bytes.append(cell[0])
                bytes.append(cell[1])
        if 'cell' in self.thenArgs.keys():
            bytes.append(self.thenArgs['cell'][0])
            bytes.append(self.thenArgs['cell'][1])
        if 'ids' in self.thenArgs.keys():
            bytes.append(len(self.thenArgs['ids']))
            for id in self.thenArgs['ids']:
                bytes.append(id)
        if 'text' in self.thenArgs.keys():
            bytes.append(len(self.thenArgs['text']))
            bytes.extend(self.thenArgs['text'].encode())
        # TODO handle spawn operation

        return bytes

    def fromBytes(self, bytes: bytearray):
        # TODO write fromBytes
        return


class scriptValidator:

    def __init__(self) -> None:
        argsRegex: str = '(\([A-Z0-9_,;\.\[\]\{\}\s]*\))'

        regex = "IF\s+("

        first = False
        for ifOp in ifOpType.__members__.items():
            if not first:
                first = True
            else:
                regex = regex + '|'
            regex = regex + ifOp[0]

        regex = regex + ")\s*"
        regex = regex + argsRegex
        regex = regex + '\s+THEN\s+('

        first = False
        for thenOp in thenOpType.__members__.items():
            if not first:
                first = True
            else:
                regex = regex + '|'
            regex = regex + thenOp[0]
        regex = regex + ')\s*'
        regex = regex + argsRegex

        self.pattern: re.Pattern = re.compile(regex, flags=re.IGNORECASE)
        pass

    def validateScript(self, scriptLine: str) -> bool:
        match = self.pattern.fullmatch(scriptLine)
        if None != match:
            if_op: str = match.group(1)
            if_args: str = match.group(2)
            then_op: str = match.group(3)
            then_args: str = match.group(4)

            script: rme_script = rme_script()
            return script.fromString(if_op, if_args, then_op, then_args)

        return False
