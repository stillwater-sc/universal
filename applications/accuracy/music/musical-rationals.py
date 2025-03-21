from dataclasses import dataclass
from typing import List, Tuple
import math
import itertools

@dataclass
class Note:
    name: str
    frequency: Tuple[int, int]  # numerator, denominator
    cents: float  # deviation from equal temperament

class HarmonicsCalculator:
    def __init__(self, base_frequency: int = 440):  # A4 = 440Hz
        self.base_frequency = base_frequency
        self.perfect_ratios = {
            "unison": (1, 1),
            "minor_second": (16, 15),
            "major_second": (9, 8),
            "minor_third": (6, 5),
            "major_third": (5, 4),
            "perfect_fourth": (4, 3),
            "tritone": (45, 32),
            "perfect_fifth": (3, 2),
            "minor_sixth": (8, 5),
            "major_sixth": (5, 3),
            "minor_seventh": (9, 5),
            "major_seventh": (15, 8),
            "octave": (2, 1)
        }
    
    def multiply_ratios(self, ratio1: Tuple[int, int], ratio2: Tuple[int, int]) -> Tuple[int, int]:
        """Multiply two frequency ratios"""
        return (ratio1[0] * ratio2[0], ratio1[1] * ratio2[1])
    
    def reduce_ratio(self, ratio: Tuple[int, int]) -> Tuple[int, int]:
        """Reduce a ratio to its simplest form"""
        def gcd(a: int, b: int) -> int:
            while b:
                a, b = b, a % b
            return a
        divisor = gcd(ratio[0], ratio[1])
        return (ratio[0] // divisor, ratio[1] // divisor)
    
    def ratio_to_cents(self, ratio: Tuple[int, int]) -> float:
        """Convert a frequency ratio to cents (1200 cents = octave)"""
        return 1200 * math.log2(ratio[0] / ratio[1])
    
    def generate_scale(self, root_ratio: Tuple[int, int], intervals: List[str]) -> List[Note]:
        """Generate a scale starting from a root note using specified intervals"""
        scale = []
        current_ratio = root_ratio
        
        for i, interval in enumerate(intervals):
            ratio = self.perfect_ratios[interval]
            current_ratio = self.reduce_ratio(self.multiply_ratios(root_ratio, ratio))
            frequency = (current_ratio[0] * self.base_frequency, current_ratio[1])
            cents = self.ratio_to_cents(current_ratio) % 1200
            et_cents = (i + 1) * 100  # Equal temperament cents
            cents_deviation = cents - et_cents
            
            scale.append(Note(
                name=interval,
                frequency=frequency,
                cents=cents_deviation
            ))
        
        return scale
    
    def find_harmonics(self, fundamental: Tuple[int, int], max_order: int = 8) -> List[Note]:
        """Generate harmonic series up to specified order"""
        harmonics = []
        for i in range(1, max_order + 1):
            ratio = (i * fundamental[0], fundamental[1])
            ratio = self.reduce_ratio(ratio)
            cents = self.ratio_to_cents(ratio) % 1200
            et_cents = round(cents / 100) * 100  # Nearest equal temperament note
            cents_deviation = cents - et_cents
            
            harmonics.append(Note(
                name=f"Harmonic {i}",
                frequency=ratio,
                cents=cents_deviation
            ))
        
        return harmonics
    
    def find_just_intervals(self, max_numerator: int = 16, max_denominator: int = 16) -> List[Note]:
        """Find all possible just intervals within limits"""
        intervals = []
        seen_ratios = set()
        
        for num in range(1, max_numerator + 1):
            for den in range(1, max_denominator + 1):
                if math.gcd(num, den) == 1:
                    ratio = (num, den)
                    cents = self.ratio_to_cents(ratio) % 1200
                    
                    if 0 < cents < 1200 and ratio not in seen_ratios:
                        seen_ratios.add(ratio)
                        et_cents = round(cents / 100) * 100
                        cents_deviation = cents - et_cents
                        
                        intervals.append(Note(
                            name=f"{num}:{den}",
                            frequency=ratio,
                            cents=cents_deviation
                        ))
        
        return sorted(intervals, key=lambda x: self.ratio_to_cents(x.frequency))

def demonstrate_capabilities():
    calc = HarmonicsCalculator()
    
    # 1. Generate a just intonation major scale
    print("Just Intonation Major Scale:")
    major_scale = calc.generate_scale((1, 1), [
        "unison", "major_second", "major_third", "perfect_fourth",
        "perfect_fifth", "major_sixth", "major_seventh"
    ])
    for note in major_scale:
        freq = note.frequency[0] / note.frequency[1] * 440
        print(f"{note.name}: {note.frequency} ({freq:.2f} Hz), deviation: {note.cents:+.2f} cents")
    print()
    
    # 2. Generate harmonic series
    print("First 8 Harmonics of A4 (440 Hz):")
    harmonics = calc.find_harmonics((1, 1))
    for harmonic in harmonics:
        freq = harmonic.frequency[0] / harmonic.frequency[1] * 440
        print(f"{harmonic.name}: {harmonic.frequency} ({freq:.2f} Hz), deviation: {harmonic.cents:+.2f} cents")
    print()
    
    # 3. Find simple just intervals
    print("Simple Just Intervals (up to 5:4):")
    intervals = calc.find_just_intervals(5, 4)
    for interval in intervals:
        cents = calc.ratio_to_cents(interval.frequency)
        print(f"Ratio {interval.name}: {cents:.2f} cents, deviation: {interval.cents:+.2f} cents")

if __name__ == "__main__":
    demonstrate_capabilities()
