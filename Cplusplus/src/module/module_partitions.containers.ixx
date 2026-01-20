// module partitions
/*
• Rules for partitions
  • Module interface partitions with the same module name
    are part of the same module
  • Partitions are not implicitly known to one another（分区之间不互相知道）
  • They do not implicitly import the module's interface.
  • Partitions may be imported only into other module units
    that belong to the same module
  • One module interface partition unit can import another
    from the same module to use the other partition's features
*/

export module containers;  // declares the primary module interface unit

// import and re-export the declarations in the module
// interface partitions :sequence and :associative
export import :sequence;
export import :associative;