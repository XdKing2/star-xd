#!/usr/bin/env python3
"""
Sudoku generator and solver.
Usage:
    python3 sudoku.py generate [easy|medium|hard]
    python3 sudoku.py solve <81-char grid, 0 for empty>
"""
import sys
import random
import json
import copy

def is_valid(board, row, col, num):
    if num in board[row]:
        return False
    if num in [board[r][col] for r in range(9)]:
        return False
    br, bc = (row // 3) * 3, (col // 3) * 3
    for r in range(br, br + 3):
        for c in range(bc, bc + 3):
            if board[r][c] == num:
                return False
    return True

def solve(board):
    for row in range(9):
        for col in range(9):
            if board[row][col] == 0:
                nums = list(range(1, 10))
                random.shuffle(nums)
                for num in nums:
                    if is_valid(board, row, col, num):
                        board[row][col] = num
                        if solve(board):
                            return True
                        board[row][col] = 0
                return False
    return True

def solve_deterministic(board):
    """Solve without randomness for user-submitted puzzles."""
    for row in range(9):
        for col in range(9):
            if board[row][col] == 0:
                for num in range(1, 10):
                    if is_valid(board, row, col, num):
                        board[row][col] = num
                        if solve_deterministic(board):
                            return True
                        board[row][col] = 0
                return False
    return True

def count_solutions(board, limit=2):
    """Count solutions up to limit (to check uniqueness)."""
    count = [0]
    def bt(b):
        if count[0] >= limit:
            return
        for r in range(9):
            for c in range(9):
                if b[r][c] == 0:
                    for n in range(1, 10):
                        if is_valid(b, r, c, n):
                            b[r][c] = n
                            bt(b)
                            b[r][c] = 0
                    return
        count[0] += 1
    bt([row[:] for row in board])
    return count[0]

def generate_puzzle(difficulty='medium'):
    # Start with empty board and fill it
    board = [[0]*9 for _ in range(9)]
    solve(board)
    solution = copy.deepcopy(board)

    # Remove cells based on difficulty
    clues = {'easy': 36, 'medium': 28, 'hard': 22}.get(difficulty, 28)
    cells_to_remove = 81 - clues

    positions = list(range(81))
    random.shuffle(positions)

    removed = 0
    for pos in positions:
        if removed >= cells_to_remove:
            break
        r, c = pos // 9, pos % 9
        backup = board[r][c]
        board[r][c] = 0
        # Quick check: still has a solution (skip full uniqueness for speed)
        test = copy.deepcopy(board)
        if solve_deterministic(test):
            removed += 1
        else:
            board[r][c] = backup

    return board, solution

def board_to_str(board):
    return ''.join(str(board[r][c]) for r in range(9) for c in range(9))

def str_to_board(s):
    if len(s) != 81:
        raise ValueError(f"Grid must be 81 chars, got {len(s)}")
    board = []
    for i in range(9):
        row = []
        for j in range(9):
            ch = s[i*9+j]
            if ch not in '0123456789':
                raise ValueError(f"Invalid char: {ch}")
            row.append(int(ch))
        board.append(row)
    return board

def format_board(board, solution=None, given=None):
    """Format board as pretty ASCII with box drawing."""
    lines = []
    lines.append('┌───────┬───────┬───────┐')
    for r in range(9):
        if r == 3 or r == 6:
            lines.append('├───────┼───────┼───────┤')
        row_str = '│'
        for c in range(9):
            val = board[r][c]
            if val == 0:
                row_str += ' ·'
            else:
                row_str += f' {val}'
            if c == 2 or c == 5:
                row_str += ' │'
        row_str += ' │'
        lines.append(row_str)
    lines.append('└───────┴───────┴───────┘')
    return '\n'.join(lines)

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print(json.dumps({'error': 'No command provided'}))
        sys.exit(1)

    cmd = sys.argv[1]

    if cmd == 'generate':
        difficulty = sys.argv[2] if len(sys.argv) > 2 else 'medium'
        if difficulty not in ('easy', 'medium', 'hard'):
            difficulty = 'medium'
        puzzle, solution = generate_puzzle(difficulty)
        clues = sum(1 for r in range(9) for c in range(9) if puzzle[r][c] != 0)
        print(json.dumps({
            'puzzle': board_to_str(puzzle),
            'solution': board_to_str(solution),
            'difficulty': difficulty,
            'clues': clues,
            'formatted_puzzle': format_board(puzzle),
            'formatted_solution': format_board(solution),
        }))

    elif cmd == 'solve':
        if len(sys.argv) < 3:
            print(json.dumps({'error': 'No grid provided'}))
            sys.exit(1)
        grid_str = sys.argv[2].replace(' ', '').replace('.', '0')
        try:
            board = str_to_board(grid_str)
            original = copy.deepcopy(board)
            empty_count = sum(1 for r in range(9) for c in range(9) if board[r][c] == 0)
            if empty_count == 0:
                print(json.dumps({'error': 'Board is already complete'}))
                sys.exit(0)
            if solve_deterministic(board):
                print(json.dumps({
                    'solved': True,
                    'puzzle': board_to_str(original),
                    'solution': board_to_str(board),
                    'filled': empty_count,
                    'formatted_puzzle': format_board(original),
                    'formatted_solution': format_board(board),
                }))
            else:
                print(json.dumps({'error': 'No solution exists for this puzzle'}))
        except ValueError as e:
            print(json.dumps({'error': str(e)}))
    else:
        print(json.dumps({'error': f'Unknown command: {cmd}'}))
        sys.exit(1)
