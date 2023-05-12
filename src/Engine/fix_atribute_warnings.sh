find ./ -type f -name '*.?pp' -exec sed -i 's/ENGINE_EXPORT class/class ENGINE_EXPORT/g' {} +
find ./ -type f -name '*.?pp' -exec sed -i 's/ENGINE_EXPORT struct/struct ENGINE_EXPORT/g' {} +
