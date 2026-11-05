function sumAll(numbers: number[]): number {
   let outer = 0; numbers.map(num => { outer += num }); return outer;} 
console.log(sumAll([1,2,3, 4]));
